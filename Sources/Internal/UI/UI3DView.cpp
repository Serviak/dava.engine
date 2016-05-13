/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"
#include "Render/RenderHelper.h"
#include "Render/OcclusionQuery.h"
#include "Core/Core.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/SnapToLandscapeControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

namespace DAVA
{
UI3DView::UI3DView(const Rect& rect)
    : UIControl(rect)
    , scene(nullptr)
    , drawToFrameBuffer(false)
    , fbScaleFactor(1.f)
    , fbRenderSize()
    , registeredInUIControlSystem(false)
{
}

UI3DView::~UI3DView()
{
    SafeRelease(frameBuffer);
    SafeRelease(scene);
}

void UI3DView::SetScene(Scene* _scene)
{
    SafeRelease(scene);

    scene = SafeRetain(_scene);

    if (scene)
    {
        float32 aspect = size.dx / size.dy;
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

Scene* UI3DView::GetScene() const
{
    return scene;
}

void UI3DView::AddControl(UIControl* control)
{
    DVASSERT(0 && "UI3DView do not support children");
}

void UI3DView::Update(float32 timeElapsed)
{
    if (scene)
        scene->Update(timeElapsed);
}

void UI3DView::Draw(const UIGeometricData& geometricData)
{
    if (!scene)
        return;

    RenderSystem2D::Instance()->Flush();

    rhi::RenderPassConfig& config = scene->GetMainPassConfig();
    const RenderSystem2D::RenderTargetPassDescriptor& currentTarget = RenderSystem2D::Instance()->GetActiveTargetDescriptor();

    Rect viewportRect = geometricData.GetUnrotatedRect();

    if (currentTarget.transformVirtualToPhysical)
        viewportRc = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(viewportRect);
    else
        viewportRc = viewportRect;

    if (drawToFrameBuffer)
    {
        // Calculate viewport for frame buffer
        viewportRc.x = 0.f;
        viewportRc.y = 0.f;
        viewportRc.dx *= fbScaleFactor;
        viewportRc.dy *= fbScaleFactor;

        PrepareFrameBuffer();

        rhi::RenderPassConfig& config = scene->GetMainPassConfig();
        config.priority = currentTarget.priority + PRIORITY_SERVICE_3D;
        config.colorBuffer[0].texture = frameBuffer->handle;
        config.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
        config.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        config.depthStencilBuffer.texture = frameBuffer->handleDepthStencil;
        config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        config.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    }
    else
    {
        if (currentTarget.transformVirtualToPhysical)
            viewportRc += VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

        config.colorBuffer[0].texture = currentTarget.colorAttachment;
        config.depthStencilBuffer.texture = currentTarget.depthAttachment.IsValid() ? currentTarget.depthAttachment : rhi::DefaultDepthBuffer;
        config.priority = currentTarget.priority + basePriority;
        config.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
        config.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        config.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    }

    bool uiDrawQueryWasOpen = FrameOcclusionQueryManager::Instance()->IsQueryOpen(FRAME_QUERY_UI_DRAW);
    if (uiDrawQueryWasOpen)
        FrameOcclusionQueryManager::Instance()->EndQuery(FRAME_QUERY_UI_DRAW);

    scene->SetMainPassViewport(viewportRc);
    scene->Draw();

    if (uiDrawQueryWasOpen)
        FrameOcclusionQueryManager::Instance()->BeginQuery(FRAME_QUERY_UI_DRAW);

    if (drawToFrameBuffer)
    {
        RenderSystem2D::Instance()->DrawTexture(frameBuffer, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, geometricData.GetUnrotatedRect(), Rect(Vector2(), fbTexSize));
    }
}

void UI3DView::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
    float32 aspect = size.dx / size.dy;

    if (scene)
    {
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

UI3DView* UI3DView::Clone()
{
    UI3DView* ui3DView = new UI3DView(GetRect());
    ui3DView->CopyDataFrom(this);
    return ui3DView;
}

void UI3DView::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);

    UI3DView* srcView = DynamicTypeCheck<UI3DView*>(srcControl);
    drawToFrameBuffer = srcView->drawToFrameBuffer;
    fbScaleFactor = srcView->fbScaleFactor;
    fbRenderSize = srcView->fbRenderSize;
    fbTexSize = srcView->fbTexSize;
}

void UI3DView::Input(UIEvent* currentInput)
{
    if (scene)
    {
        scene->Input(currentInput);
    }

    UIControl::Input(currentInput);
}

void UI3DView::SetDrawToFrameBuffer(bool enable)
{
    drawToFrameBuffer = enable;

    if (!enable)
    {
        SafeRelease(frameBuffer);
    }
}

void UI3DView::SetFrameBufferScaleFactor(float32 scale)
{
    fbScaleFactor = scale;
}

void UI3DView::PrepareFrameBuffer()
{
    DVASSERT(scene);

    fbRenderSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(GetSize()) * fbScaleFactor;

    if (frameBuffer == nullptr || frameBuffer->GetWidth() < fbRenderSize.dx || frameBuffer->GetHeight() < fbRenderSize.dy)
    {
        SafeRelease(frameBuffer);
        int32 dx = static_cast<int32>(fbRenderSize.dx);
        int32 dy = static_cast<int32>(fbRenderSize.dy);
        frameBuffer = Texture::CreateFBO(dx, dy, FORMAT_RGBA8888, true);
    }

    Vector2 fbSize = Vector2(static_cast<float32>(frameBuffer->GetWidth()), static_cast<float32>(frameBuffer->GetHeight()));

    fbTexSize = fbRenderSize / fbSize;
}
}