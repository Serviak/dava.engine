#include "Base/BaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/Engine.h"
#include "Render/RenderBase.h"
#include "Render/Renderer.h"
#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"
#include "Input/InputSystem.h"
#include "Input/InputEvent.h"
#include "Input/Mouse.h"
#include "Input/Keyboard.h"
#include "Debug/Private/ImGui.h"
#include "DeviceManager/DeviceManager.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "Render/RHI/rhi_ShaderSource.h"
#include "Render/DynamicBufferAllocator.h"
#include "Time/SystemTimer.h"
#include "Debug/DVAssert.h"

namespace ImGuiImplDetails
{
static bool initialized = false;

static rhi::HPipelineState pipelineStatePC, pipelineStatePTC;
static rhi::HConstBuffer constBufferPC, constBufferPTC;
static rhi::HDepthStencilState depthState;
static DAVA::uint32 vertexLayout = 0;
static rhi::HSamplerState fontSamplerState;
static rhi::HTextureSet fontTextureSet;
static rhi::HTexture fontTexture;

static DAVA::TrackedObject* trackedObject = nullptr;
static DAVA::uint32 inputHandlerToken = 0;

static DAVA::Size2i framebufferSize = { 0, 0 };

static const char* IMGUI_RENDER_PASS_MARKER_NAME = "ImGuiRenderPass";

static const char* vprogPC =
"vertex_in\n"
"{\n"
"    float2 pos : TEXCOORD0;\n"
"    float4 color : COLOR0;\n"
"};\n"
"vertex_out\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR0;\n"
"};\n"
"[material][unique] property float4x4 XForm;"
"vertex_out vp_main( vertex_in input )\n"
"{\n"
"    vertex_out output;\n"
"    output.position = mul( float4(input.pos.x,-input.pos.y,0.0,1.0), XForm );\n"
"    output.color = input.color;\n"
"    return output;\n"
"}\n";

static const char* fprogPC =
"fragment_in\n"
"{\n"
"    float4 color : COLOR0;\n"
"};\n"
"fragment_out\n"
"{\n"
"    float4 color : SV_TARGET0;\n"
"};\n"
"\n"
"fragment_out\n"
"fp_main( fragment_in input )\n"
"{\n"
"    fragment_out output;\n"
"    output.color = input.color;\n"
"    return output;\n"
"}\n"
"blending { src=src_alpha dst=inv_src_alpha }\n";

static const char* vprogPTC =
"vertex_in\n"
"{\n"
"    float2 pos : TEXCOORD0;\n"
"    float2 uv : TEXCOORD1;\n"
"    float4 color : COLOR0;\n"
"};\n"
"vertex_out\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float2 uv : TEXCOORD0;\n"
"    float4 color : COLOR0;\n"
"};\n"
"[material][unique] property float4x4 XForm;"
"vertex_out vp_main( vertex_in input )\n"
"{\n"
"    vertex_out output;\n"
"    output.position = mul(float4(input.pos.x,-input.pos.y,0.0,1.0), XForm);\n"
"    output.uv = input.uv;\n"
"    output.color = input.color;\n"
"    return output;\n"
"}\n";

static const char* fprogPTC =
"fragment_in\n"
"{\n"
"    float2 uv    : TEXCOORD0;\n"
"    float4 color : COLOR0;\n"
"};\n"
"fragment_out\n"
"{\n"
"    float4 color : SV_TARGET0;\n"
"};\n"
"\n"
"uniform sampler2D Image;\n"
"\n"
"fragment_out\n"
"fp_main( fragment_in input )\n"
"{\n"
"    fragment_out output;\n"
"    float4       image = tex2D( Image, input.uv );"
"    output.color = image * input.color;\n"
"    return output;\n"
"}\n"
"blending { src=src_alpha dst=inv_src_alpha }\n";

void ImGuiDrawFn(ImDrawData* data)
{
    if (!data->Valid)
        return;

    rhi::RenderPassConfig passConfig;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = DAVA::PRIORITY_MAIN_2D - 10;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = DAVA::uint32(framebufferSize.dx);
    passConfig.viewport.height = DAVA::uint32(framebufferSize.dy);

    DAVA::Matrix4 ortho(
    2.0f / framebufferSize.dx, 0.0f, 0.0f, -1.0f,
    0.0f, 2.0f / framebufferSize.dy, 0.0f, 1.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

    if (rhi::DeviceCaps().isCenterPixelMapping)
    {
        ortho._03 -= 1.0f / framebufferSize.dx;
        ortho._13 -= 1.0f / framebufferSize.dy;
    }

    ortho.Transpose();

    rhi::UpdateConstBuffer4fv(constBufferPC, 0, ortho.data, 4);
    rhi::UpdateConstBuffer4fv(constBufferPTC, 0, ortho.data, 4);

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    for (DAVA::int32 i = 0; i < data->CmdListsCount; ++i)
    {
        ImDrawList* cmdList = data->CmdLists[i];

        DAVA::DynamicBufferAllocator::AllocResultIB ib = DAVA::DynamicBufferAllocator::AllocateIndexBuffer(cmdList->IdxBuffer.size());
        Memcpy(ib.data, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.size() * sizeof(DAVA::uint16));

        DAVA::DynamicBufferAllocator::AllocResultVB vb = DAVA::DynamicBufferAllocator::AllocateVertexBuffer(sizeof(ImDrawVert), cmdList->VtxBuffer.size());

        if (rhi::HostApi() == rhi::RHI_DX9)
        {
            ImDrawVert* vxPtr = reinterpret_cast<ImDrawVert*>(vb.data);
            for (ImDrawVert& vx : cmdList->VtxBuffer)
            {
                vxPtr->pos = vx.pos;
                vxPtr->uv = vx.uv;
                vxPtr->col = rhi::NativeColorRGBA(vx.col);
                ++vxPtr;
            }
        }
        else
        {
            Memcpy(vb.data, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.size() * sizeof(ImDrawVert));
        }

        rhi::Packet packet;
        packet.vertexStreamCount = 1;
        packet.vertexStream[0] = vb.buffer;
        packet.vertexCount = vb.allocatedVertices;
        packet.baseVertex = vb.baseVertex;
        packet.startIndex = ib.baseIndex;
        packet.indexBuffer = ib.buffer;
        packet.cullMode = rhi::CULL_NONE;
        packet.depthStencilState = depthState;
        packet.vertexConstCount = 1;
        packet.fragmentConstCount = 0;
        packet.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        packet.vertexLayoutUID = ImGuiImplDetails::vertexLayout;

        for (ImDrawCmd& cmd : cmdList->CmdBuffer)
        {
            if (cmd.TextureId)
            {
                packet.renderPipelineState = pipelineStatePTC;
                packet.vertexConst[0] = constBufferPTC;

                packet.textureSet = *reinterpret_cast<rhi::HTextureSet*>(&cmd.TextureId);
                packet.samplerState = fontSamplerState;
            }
            else
            {
                packet.renderPipelineState = pipelineStatePC;
                packet.vertexConst[0] = constBufferPC;
                packet.textureSet = rhi::HTextureSet();
                packet.samplerState = rhi::HSamplerState();
            }

            packet.primitiveCount = cmd.ElemCount / 3;

            packet.scissorRect.x = DAVA::uint16(cmd.ClipRect.x);
            packet.scissorRect.y = DAVA::uint16(cmd.ClipRect.y);
            packet.scissorRect.width = DAVA::uint16(cmd.ClipRect.z - cmd.ClipRect.x);
            packet.scissorRect.height = DAVA::uint16(cmd.ClipRect.w - cmd.ClipRect.y);

            if (packet.scissorRect.width && packet.scissorRect.height && (packet.scissorRect.width != framebufferSize.dx || packet.scissorRect.height != framebufferSize.dy))
                packet.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;
            else
                packet.options &= ~rhi::Packet::OPT_OVERRIDE_SCISSOR;

            rhi::AddPacket(packetList, packet);

            packet.startIndex += cmd.ElemCount;
        }
    }

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

} //ns Details

namespace ImGui
{
using DAVA::uint8;
using DAVA::int32;
using DAVA::uint32;
using DAVA::UIEvent;
using DAVA::float32;
using DAVA::Vector2;
using DAVA::eInputDevices;
using DAVA::eInputElements;

void Initialize()
{
    DVASSERT(!ImGuiImplDetails::initialized);

    ImGuiIO& io = ImGui::GetIO();
    io.RenderDrawListsFn = ImGuiImplDetails::ImGuiDrawFn;

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.KeyMap[ImGuiKey_Tab] = int32(eInputElements::KB_TAB);
    io.KeyMap[ImGuiKey_LeftArrow] = int32(eInputElements::KB_LEFT);
    io.KeyMap[ImGuiKey_RightArrow] = int32(eInputElements::KB_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow] = int32(eInputElements::KB_UP);
    io.KeyMap[ImGuiKey_DownArrow] = int32(eInputElements::KB_DOWN);
    io.KeyMap[ImGuiKey_PageUp] = int32(eInputElements::KB_PAGEUP);
    io.KeyMap[ImGuiKey_PageDown] = int32(eInputElements::KB_PAGEDOWN);
    io.KeyMap[ImGuiKey_Home] = int32(eInputElements::KB_HOME);
    io.KeyMap[ImGuiKey_End] = int32(eInputElements::KB_END);
    io.KeyMap[ImGuiKey_Delete] = int32(eInputElements::KB_DELETE);
    io.KeyMap[ImGuiKey_Backspace] = int32(eInputElements::KB_BACKSPACE);
    io.KeyMap[ImGuiKey_Enter] = int32(eInputElements::KB_ENTER);
    io.KeyMap[ImGuiKey_Escape] = int32(eInputElements::KB_ESCAPE);
    io.KeyMap[ImGuiKey_A] = int32(eInputElements::KB_A);
    io.KeyMap[ImGuiKey_C] = int32(eInputElements::KB_C);
    io.KeyMap[ImGuiKey_V] = int32(eInputElements::KB_V);
    io.KeyMap[ImGuiKey_X] = int32(eInputElements::KB_X);
    io.KeyMap[ImGuiKey_Y] = int32(eInputElements::KB_Y);
    io.KeyMap[ImGuiKey_Z] = int32(eInputElements::KB_Z);

    //vertex layouts
    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 2);
    vLayout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    ImGuiImplDetails::vertexLayout = rhi::VertexLayout::UniqueId(vLayout);

    //font sampler-state
    rhi::SamplerState::Descriptor ss_desc;
    ss_desc.fragmentSamplerCount = 1;
    ss_desc.fragmentSampler[0].minFilter = rhi::TEXFILTER_LINEAR;
    ss_desc.fragmentSampler[0].magFilter = rhi::TEXFILTER_LINEAR;
    ss_desc.fragmentSampler[0].mipFilter = rhi::TEXMIPFILTER_NONE;
    ImGuiImplDetails::fontSamplerState = rhi::AcquireSamplerState(ss_desc);

    //depth state
    rhi::DepthStencilState::Descriptor ds_desc;
    ds_desc.depthTestEnabled = false;
    ds_desc.depthWriteEnabled = false;
    ImGuiImplDetails::depthState = rhi::AcquireDepthStencilState(ds_desc);

    //pc pipeline state
    rhi::ShaderSource vp_pc;
    rhi::ShaderSource fp_pc;

    if (vp_pc.Construct(rhi::PROG_VERTEX, ImGuiImplDetails::vprogPC) && fp_pc.Construct(rhi::PROG_FRAGMENT, ImGuiImplDetails::fprogPC))
    {
        rhi::PipelineState::Descriptor ps_desc;

        ps_desc.vertexLayout = vp_pc.ShaderVertexLayout();
        ps_desc.vprogUid = DAVA::FastName("imgui.vp.pc");
        ps_desc.fprogUid = DAVA::FastName("imgui.fp.pc");
        ps_desc.blending = fp_pc.Blending();

        const std::string& vp_bin = vp_pc.GetSourceCode(rhi::HostApi());
        const std::string& fp_bin = fp_pc.GetSourceCode(rhi::HostApi());

        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, ps_desc.vprogUid, vp_bin.c_str(), unsigned(vp_bin.length()));
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, ps_desc.fprogUid, fp_bin.c_str(), unsigned(fp_bin.length()));

        ImGuiImplDetails::pipelineStatePC = rhi::AcquireRenderPipelineState(ps_desc);
        rhi::CreateVertexConstBuffers(ImGuiImplDetails::pipelineStatePC, 1, &ImGuiImplDetails::constBufferPC);
    }

    //ptc pipeline-state
    rhi::ShaderSource vp_ptc;
    rhi::ShaderSource fp_ptc;

    if (vp_ptc.Construct(rhi::PROG_VERTEX, ImGuiImplDetails::vprogPTC) && fp_ptc.Construct(rhi::PROG_FRAGMENT, ImGuiImplDetails::fprogPTC))
    {
        rhi::PipelineState::Descriptor ps_desc;

        ps_desc.vertexLayout = vp_ptc.ShaderVertexLayout();
        ps_desc.vprogUid = DAVA::FastName("imgui.vp.ptc");
        ps_desc.fprogUid = DAVA::FastName("imgui.fp.ptc");
        ps_desc.blending = fp_ptc.Blending();

        const std::string& vp_bin = vp_ptc.GetSourceCode(rhi::HostApi());
        const std::string& fp_bin = fp_ptc.GetSourceCode(rhi::HostApi());

        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, ps_desc.vprogUid, vp_bin.c_str(), unsigned(vp_bin.length()));
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, ps_desc.fprogUid, fp_bin.c_str(), unsigned(fp_bin.length()));

        ImGuiImplDetails::pipelineStatePTC = rhi::AcquireRenderPipelineState(ps_desc);
        rhi::CreateVertexConstBuffers(ImGuiImplDetails::pipelineStatePTC, 1, &ImGuiImplDetails::constBufferPTC);
    }

    ImGuiImplDetails::trackedObject = new DAVA::TrackedObject();
    DAVA::Engine::Instance()->beginFrame.Connect(ImGuiImplDetails::trackedObject, &OnFrameBegin);
    DAVA::Engine::Instance()->endFrame.Connect(ImGuiImplDetails::trackedObject, &OnFrameEnd);

    ImGuiImplDetails::inputHandlerToken = DAVA::InputSystem::Instance()->AddHandler
                                          (eInputDevices::TOUCH_SURFACE | eInputDevices::MOUSE | eInputDevices::KEYBOARD,
                                           DAVA::MakeFunction<bool, const DAVA::InputEvent&>(&OnInput));

    ImGuiImplDetails::initialized = true;
}

bool IsInitialized()
{
    return ImGuiImplDetails::initialized;
}

void OnFrameBegin()
{
    if (!ImGuiImplDetails::initialized)
        return;

    ImGui::GetIO().DeltaTime = DAVA::SystemTimer::GetFrameDelta();

    ImGuiImplDetails::framebufferSize.dx = int32(DAVA::Renderer::GetFramebufferWidth());
    ImGuiImplDetails::framebufferSize.dy = int32(DAVA::Renderer::GetFramebufferHeight());

    ImGui::GetIO().DisplaySize.x = float32(ImGuiImplDetails::framebufferSize.dx);
    ImGui::GetIO().DisplaySize.y = float32(ImGuiImplDetails::framebufferSize.dy);

    //check whether to recreate font texture
    for (ImFont* font : ImGui::GetIO().Fonts->Fonts)
    {
        if (!font->IsLoaded())
        {
            if (ImGuiImplDetails::fontTexture.IsValid())
                rhi::DeleteTexture(ImGuiImplDetails::fontTexture);

            if (ImGuiImplDetails::fontTextureSet)
                rhi::ReleaseTextureSet(ImGuiImplDetails::fontTextureSet);

            ImGuiImplDetails::fontTexture = rhi::HTexture();
            ImGuiImplDetails::fontTextureSet = rhi::HTextureSet();

            break;
        }
    }

    //create font texture if needed
    if (!ImGuiImplDetails::fontTexture.IsValid())
    {
        ImGuiIO& io = ImGui::GetIO();
        uint8* pixels;
        int32 width, height, bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

        rhi::Texture::Descriptor tex_desc;
        tex_desc.width = uint32(width);
        tex_desc.height = uint32(height);
        tex_desc.format = rhi::TextureFormat::TEXTURE_FORMAT_R8G8B8A8;
        tex_desc.initialData[0] = pixels;
        ImGuiImplDetails::fontTexture = rhi::CreateTexture(tex_desc);

        rhi::TextureSetDescriptor set_desc;
        set_desc.fragmentTextureCount = 1;
        set_desc.fragmentTexture[0] = ImGuiImplDetails::fontTexture;
        ImGuiImplDetails::fontTextureSet = rhi::AcquireTextureSet(set_desc);

        io.Fonts->SetTexID(*reinterpret_cast<void**>(&ImGuiImplDetails::fontTextureSet));
        io.Fonts->ClearTexData();
    }

    //check if need restore font texture after reset
    if (ImGuiImplDetails::fontTexture.IsValid() && rhi::NeedRestoreTexture(ImGuiImplDetails::fontTexture))
    {
        ImGuiIO& io = ImGui::GetIO();

        uint8* pixels;
        int32 width, height, bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
        rhi::UpdateTexture(ImGuiImplDetails::fontTexture, pixels, 0);
        io.Fonts->SetTexID(*reinterpret_cast<void**>(&ImGuiImplDetails::fontTextureSet));
        io.Fonts->ClearTexData();
    }

    ImGui::NewFrame();
}

void OnFrameEnd()
{
    if (ImGuiImplDetails::initialized)
        ImGui::Render();
}

bool OnInput(const DAVA::InputEvent& input)
{
    using namespace DAVA;

    if (!ImGuiImplDetails::initialized)
        return false;

    ImGuiIO& io = ImGui::GetIO();

    if (input.deviceType == eInputDevices::MOUSE)
    {
        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();

        DAVA::AnalogElementState mousePos = mouse->GetPosition();
        io.MousePos.x = mousePos.x;
        io.MousePos.y = mousePos.y;

        if (IsMouseButtonInputElement(input.elementId))
        {
            uint32 mouseButtonIndex = input.elementId - eInputElements::MOUSE_FIRST_BUTTON;
            io.MouseDown[mouseButtonIndex] = input.digitalState.IsPressed();
        }
        else if (input.elementId == eInputElements::MOUSE_WHEEL)
        {
            DAVA::AnalogElementState mouseWheelDelta = mouse->GetWheelDelta();
            io.MouseWheel = mouseWheelDelta.y;
        }
    }
    else if (input.deviceType == eInputDevices::KEYBOARD)
    {
        if (input.keyboardEvent.charCode > 0)
        {
            io.AddInputCharacter(ImWchar(input.keyboardEvent.charCode));
        }
        else
        {
            bool keyIsPressed = input.digitalState.IsPressed();

            io.KeysDown[static_cast<size_t>(input.elementId) - eInputElements::KB_FIRST] = keyIsPressed;

            if (IsKeyboardModifierInputElement(input.elementId))
            {
                if (input.elementId == eInputElements::KB_LCTRL || input.elementId == eInputElements::KB_RCTRL)
                {
                    io.KeyCtrl = keyIsPressed;
                }
                else if (input.elementId == eInputElements::KB_LALT || input.elementId == eInputElements::KB_RALT)
                {
                    io.KeyAlt = keyIsPressed;
                }
                else if (input.elementId == eInputElements::KB_LCMD || input.elementId == eInputElements::KB_RCMD)
                {
                    io.KeySuper = keyIsPressed;
                }
            }
        }
    }

    return (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput);
}

void Uninitialize()
{
    if (ImGuiImplDetails::initialized)
    {
        rhi::DeleteTexture(ImGuiImplDetails::fontTexture);
        rhi::ReleaseTextureSet(ImGuiImplDetails::fontTextureSet);
        rhi::ReleaseRenderPipelineState(ImGuiImplDetails::pipelineStatePC);
        rhi::ReleaseRenderPipelineState(ImGuiImplDetails::pipelineStatePTC);
        rhi::ReleaseDepthStencilState(ImGuiImplDetails::depthState);
        rhi::DeleteConstBuffer(ImGuiImplDetails::constBufferPC);
        rhi::DeleteConstBuffer(ImGuiImplDetails::constBufferPTC);

        ImGui::Shutdown();

        DAVA::InputSystem::Instance()->RemoveHandler(ImGuiImplDetails::inputHandlerToken);
        ImGuiImplDetails::inputHandlerToken = 0;

        SafeDelete(ImGuiImplDetails::trackedObject);

        ImGuiImplDetails::initialized = false;
    }
}

} //ns ImGui
