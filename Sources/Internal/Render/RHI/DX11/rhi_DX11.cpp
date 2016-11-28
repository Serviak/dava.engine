#include "rhi_DX11.h"
#include "../Common/rhi_BackendImpl.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Core/Core.h"
using DAVA::Logger;

#include "_dx11.h"
#include "../rhi_Type.h"
#include "../Common/dbg_StatSet.h"
#include "../Common/rhi_Utils.h"
#include "../Common/rhi_CommonImpl.h"
#include "../Common/RenderLoop.h"
#include "../Common/FrameLoop.h"

#include <vector>
#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "uap_dx11.h"
#include "Platform/DeviceInfo.h"
#endif

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace rhi
{
static Dispatch DispatchDX11 = {};
static ResetParam resetParams;
static DAVA::Mutex resetParamsSync;

const UINT DX11_BackBuffersCount = 3; // TODO : make shared variable across DX11 code
const DXGI_FORMAT DX11_BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

//------------------------------------------------------------------------------

static Api dx11_HostApi()
{
    return RHI_DX11;
}

//------------------------------------------------------------------------------

static bool dx11_NeedRestoreResources()
{
    return false;
}

//------------------------------------------------------------------------------

static bool dx11_TextureFormatSupported(TextureFormat format, ProgType)
{
    bool supported = false;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R8:
    case TEXTURE_FORMAT_R16:
    case TEXTURE_FORMAT_DXT1:
    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
        supported = true;
        break;
    }

    return supported;
}

//------------------------------------------------------------------------------

static bool _IsValidIntelCardDX11(uint32 vendor_id, uint32 device_id)
{
    return ((vendor_id == 0x8086) && // Intel Architecture
            // These guys are prehistoric :)
            (device_id == 0x2572) || // 865G
            (device_id == 0x3582) || // 855GM
            (device_id == 0x2562) || // 845G
            (device_id == 0x3577) || // 830M
            // These are from 2005 and later
            (device_id == 0x2A02) || // GM965 Device 0
            (device_id == 0x2A03) || // GM965 Device 1
            (device_id == 0x29A2) || // G965 Device 0
            (device_id == 0x29A3) || // G965 Device 1
            (device_id == 0x27A2) || // 945GM Device 0
            (device_id == 0x27A6) || // 945GM Device 1
            (device_id == 0x2772) || // 945G Device 0
            (device_id == 0x2776) || // 945G Device 1
            (device_id == 0x2592) || // 915GM Device 0
            (device_id == 0x2792) || // 915GM Device 1
            (device_id == 0x2582) || // 915G Device 0
            (device_id == 0x2782) // 915G Device 1
            );
}

//------------------------------------------------------------------------------

static void dx11_Uninitialize()
{
    QueryBufferDX11::ReleaseQueryPool();
    PerfQueryDX11::ReleasePerfQueryPool();
}

void DX11_CreateSizeDependentResources(UINT w, UINT h)
{
    HRESULT hr = _D3D11_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&_D3D11_SwapChainBuffer));
    DX11_ProcessCallResult(hr, "_D3D11_SwapChain->GetBuffer", __FILE__, __LINE__);

    D3D11_TEXTURE2D_DESC ds_desc = {};
    ds_desc.Width = w;
    ds_desc.Height = h;
    ds_desc.MipLevels = 1;
    ds_desc.ArraySize = 1;
    ds_desc.Format = (_D3D11_FeatureLevel == D3D_FEATURE_LEVEL_11_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    ds_desc.SampleDesc.Count = 1;
    ds_desc.Usage = D3D11_USAGE_DEFAULT;
    ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DX11DeviceCommand(DX11Command::CREATE_TEXTURE_2D, &ds_desc, 0, &_D3D11_DepthStencilBuffer);
    DX11DeviceCommand(DX11Command::CREATE_RENDER_TARGET_VIEW, _D3D11_SwapChainBuffer, 0, &_D3D11_RenderTargetView);
    DX11DeviceCommand(DX11Command::CREATE_DEPTH_STENCIL_VIEW, _D3D11_DepthStencilBuffer, 0, &_D3D11_DepthStencilView);
}

static void ResizeSwapchain()
{
    DAVA::LockGuard<DAVA::Mutex> lock(resetParamsSync);

#if defined(__DAVAENGINE_WIN_UAP__)

    resize_swapchain_uap(resetParams.width, resetParams.height, resetParams.scaleX, resetParams.scaleY);

#else
    DAVA::SafeRelease(_D3D11_RenderTargetView);
    DAVA::SafeRelease(_D3D11_DepthStencilBuffer);
    DAVA::SafeRelease(_D3D11_DepthStencilView);
    DAVA::SafeRelease(_D3D11_SwapChainBuffer);
    HRESULT hr = _D3D11_SwapChain->ResizeBuffers(DX11_BackBuffersCount, resetParams.width, resetParams.height, DX11_BackBufferFormat, 0);
    DX11_ProcessCallResult(hr, "_D3D11_SwapChain->ResizeBuffers", __FILE__, __LINE__);
    DX11_CreateSizeDependentResources(resetParams.width, resetParams.height);
#endif
}

static void dx11_ResetBlock()
{
    if (_DX11_UseHardwareCommandBuffers)
    {
        DAVA::LockGuard<DAVA::Mutex> lock(_D3D11_SecondaryContextSync);

        _D3D11_SecondaryContext->ClearState();
        ID3D11CommandList* commandList = nullptr;
        CHECK_HR(_D3D11_SecondaryContext->FinishCommandList(FALSE, &commandList));
        DAVA::SafeRelease(commandList);
        _D3D11_SecondaryContext->Release();

        DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
    }
    else
    {
        rhi::ConstBufferDX11::InvalidateAll();
    }

    ID3D11RenderTargetView* view[] = { nullptr };
    _D3D11_ImmediateContext->OMSetRenderTargets(1, view, nullptr);
    ResizeSwapchain();
}

static void dx11_Reset(const ResetParam& param)
{
    {
        DAVA::LockGuard<DAVA::Mutex> lock(resetParamsSync);
        resetParams = param;
    }
    RenderLoop::SetResetPending();
}

//------------------------------------------------------------------------------

static void dx11_SuspendRendering()
{
#if defined(__DAVAENGINE_WIN_UAP__)
    FrameLoop::RejectFrames();

    _GUID uuid = __uuidof(IDXGIDevice3);
    IDXGIDevice3* dxgiDevice3 = nullptr;
    HRESULT hr = DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&dxgiDevice3));
    if (SUCCEEDED(hr))
    {
        _D3D11_ImmediateContext->ClearState();
        dxgiDevice3->Trim();
        dxgiDevice3->Release();
    }
#endif
}

static void dx11_SynchronizeCPUGPU(uint64* cpuTimestamp, uint64* gpuTimestamp)
{
    DX11Command cmd(DX11Command::SYNC_CPU_GPU, cpuTimestamp, gpuTimestamp);
    ExecDX11(&cmd, 1);
}

//------------------------------------------------------------------------------

#if !defined(__DAVAENGINE_WIN_UAP__)
void InitDeviceAndSwapChain()
{
    HRESULT hr;
#if RHI_DX11__FORCE_9X_PROFILE
    D3D_FEATURE_LEVEL feature[] = { D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
#else
    D3D_FEATURE_LEVEL feature[] = { /*D3D_FEATURE_LEVEL_11_1, */ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_1 };
#endif
    IDXGIAdapter* defAdapter = NULL;

    // enumerate adapters
    {
        IDXGIFactory* factory = NULL;
        std::vector<IDXGIAdapter*> adapter;
        const uint32 preferredVendorID[] =
        {
          0x10DE, // nVIDIA
          0x1002 // ATI
        };

        if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&factory)))
        {
            IDXGIAdapter* a = NULL;

            for (UINT i = 0; factory->EnumAdapters(i, &a) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                adapter.push_back(a);
            }
        }

        Logger::Info("detected GPUs (%u) :", adapter.size());
        for (uint32 i = 0; i != adapter.size(); ++i)
        {
            DXGI_ADAPTER_DESC desc = { 0 };

            if (SUCCEEDED(adapter[i]->GetDesc(&desc)))
            {
                char info[128];

                ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, info, countof(info) - 1, NULL, NULL);

                Logger::Info("  adapter[%u]  \"%s\"  vendor= %04X  device= %04X", i, info, desc.VendorId, desc.DeviceId);

                if (!defAdapter)
                {
                    for (uint32 k = 0; k != countof(preferredVendorID); ++k)
                    {
                        if (desc.VendorId == preferredVendorID[k])
                        {
                            defAdapter = adapter[i];
                            break;
                        }
                    }
                }
                //if( desc.VendorId == 0x8086 )
                //    defAdapter = adapter[i];
            }
        }
    }


#if 0
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    flags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#endif

    DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
    swapchain_desc.BufferDesc.Width = _DX11_InitParam.width;
    swapchain_desc.BufferDesc.Height = _DX11_InitParam.height;
    swapchain_desc.BufferDesc.Format = DX11_BackBufferFormat;
    swapchain_desc.BufferDesc.RefreshRate.Numerator = 0;
    swapchain_desc.BufferDesc.RefreshRate.Denominator = 0;
    swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.SampleDesc.Quality = 0;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = DX11_BackBuffersCount;
    swapchain_desc.OutputWindow = (HWND)_DX11_InitParam.window;
    swapchain_desc.Windowed = (_DX11_InitParam.fullScreen) ? FALSE : TRUE;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchain_desc.Flags = 0;

    DWORD deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

    hr = D3D11CreateDeviceAndSwapChain(defAdapter, (defAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, NULL,
                                       deviceFlags, feature, countof(feature), D3D11_SDK_VERSION, &swapchain_desc,
                                       &_D3D11_SwapChain, &_D3D11_Device, &_D3D11_FeatureLevel, &_D3D11_ImmediateContext);

    if (FAILED(hr))
    {
        // fall back to 'default' adapter
        hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, feature, countof(feature),
                                           D3D11_SDK_VERSION, &swapchain_desc, &_D3D11_SwapChain, &_D3D11_Device,
                                           &_D3D11_FeatureLevel, &_D3D11_ImmediateContext);
    }

    if (SUCCEEDED(hr))
    {
        IDXGIDevice* dxgiDevice = nullptr;
        IDXGIAdapter* dxgiAdapter = nullptr;
        _GUID uuid = __uuidof(IDXGIDevice);
        hr = DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter)))
            {
                DXGI_ADAPTER_DESC desc = { 0 };

                if (SUCCEEDED(dxgiAdapter->GetDesc(&desc)))
                {
                    ::WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, desc.Description, -1, MutableDeviceCaps::Get().deviceDescription,
                                          countof(MutableDeviceCaps::Get().deviceDescription), NULL, NULL);
                    Logger::Info("using adapter  \"%s\"  vendor= %04X  device= %04X",
                                 MutableDeviceCaps::Get().deviceDescription, desc.VendorId, desc.DeviceId);
                }
            }
        }

        uuid = __uuidof(IDXGIDevice);
        hr = DX11DeviceCommand(DX11Command::QUERY_INTERFACE, &uuid, (void**)(&_D3D11_Debug));
        hr = _D3D11_ImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&_D3D11_UserAnnotation));

        DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
        DX11_CreateSizeDependentResources(_DX11_InitParam.width, _DX11_InitParam.height);
    }
}
#endif

void dx11_InitCaps()
{
    MutableDeviceCaps::Get().is32BitIndicesSupported = true;
    MutableDeviceCaps::Get().isFramebufferFetchSupported = true;
    MutableDeviceCaps::Get().isVertexTextureUnitsSupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_10_0);
    MutableDeviceCaps::Get().isUpperLeftRTOrigin = true;
    MutableDeviceCaps::Get().isZeroBaseClipRange = true;
    MutableDeviceCaps::Get().isCenterPixelMapping = false;
    MutableDeviceCaps::Get().isInstancingSupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_9_2);
    MutableDeviceCaps::Get().isPerfQuerySupported = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_9_2);
    MutableDeviceCaps::Get().maxAnisotropy = D3D11_REQ_MAXANISOTROPY;

#if defined(__DAVAENGINE_WIN_UAP__)
    if (DAVA::DeviceInfo::GetPlatform() == DAVA::DeviceInfo::ePlatform::PLATFORM_PHONE_WIN_UAP)
    {
        // explicitly disable multisampling support on win phones
        MutableDeviceCaps::Get().maxSamples = 1;
    }
    else
#endif
    {
        MutableDeviceCaps::Get().maxSamples = DX11_GetMaxSupportedMultisampleCount(_D3D11_Device);
    }

    //Some drivers returns untrue DX feature-level, so check it manually
    if (MutableDeviceCaps::Get().isPerfQuerySupported)
    {
        ID3D11Query* freqQuery = nullptr;
        D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP_DISJOINT };
        _D3D11_Device->CreateQuery(&desc, &freqQuery);
        MutableDeviceCaps::Get().isPerfQuerySupported = (freqQuery != nullptr);
        DAVA::SafeRelease(freqQuery);
    }

    D3D11_FEATURE_DATA_THREADING threadingData = {};
    _D3D11_Device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingData, sizeof(threadingData));
    _DX11_UseHardwareCommandBuffers = threadingData.DriverCommandLists && threadingData.DriverConcurrentCreates;

    DAVA::Logger::Info("DX11: hardware command buffers enabled: %d", static_cast<int32>(_DX11_UseHardwareCommandBuffers));
}

//------------------------------------------------------------------------------

void dx11_InitContext()
{
    _DX11_RenderThreadId = GetCurrentThreadId();

#if defined(__DAVAENGINE_WIN_UAP__)
    init_device_and_swapchain_uap(_DX11_InitParam.window);
    get_device_description(MutableDeviceCaps::Get().deviceDescription);
    DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &_D3D11_SecondaryContext);
#else
    InitDeviceAndSwapChain();
#endif

    dx11_InitCaps();

    if (!_DX11_UseHardwareCommandBuffers)
    {
        ConstBufferDX11::InitializeRingBuffer(_DX11_InitParam.shaderConstRingBufferSize);
    }
}

bool dx11_CheckSurface()
{
    return true;
}

//------------------------------------------------------------------------------

void dx11_Initialize(const InitParam& param)
{
    _DX11_InitParam = param;

    VertexBufferDX11::SetupDispatch(&DispatchDX11);
    IndexBufferDX11::SetupDispatch(&DispatchDX11);
    QueryBufferDX11::SetupDispatch(&DispatchDX11);
    PerfQueryDX11::SetupDispatch(&DispatchDX11);
    TextureDX11::SetupDispatch(&DispatchDX11);
    PipelineStateDX11::SetupDispatch(&DispatchDX11);
    ConstBufferDX11::SetupDispatch(&DispatchDX11);
    DepthStencilStateDX11::SetupDispatch(&DispatchDX11);
    SamplerStateDX11::SetupDispatch(&DispatchDX11);
    RenderPassDX11::SetupDispatch(&DispatchDX11);
    CommandBufferDX11::SetupDispatch(&DispatchDX11);

    DispatchDX11.impl_Uninitialize = &dx11_Uninitialize;
    DispatchDX11.impl_Reset = &dx11_Reset;
    DispatchDX11.impl_HostApi = &dx11_HostApi;
    DispatchDX11.impl_TextureFormatSupported = &dx11_TextureFormatSupported;
    DispatchDX11.impl_NeedRestoreResources = &dx11_NeedRestoreResources;

    DispatchDX11.impl_InitContext = &dx11_InitContext;
    DispatchDX11.impl_ValidateSurface = &dx11_CheckSurface;
    DispatchDX11.impl_FinishRendering = &dx11_SuspendRendering;
    DispatchDX11.impl_ResetBlock = &dx11_ResetBlock;
    DispatchDX11.impl_SyncCPUGPU = &dx11_SynchronizeCPUGPU;

    SetDispatchTable(DispatchDX11);

    if (param.maxVertexBufferCount)
        VertexBufferDX11::Init(param.maxVertexBufferCount);
    if (param.maxIndexBufferCount)
        IndexBufferDX11::Init(param.maxIndexBufferCount);
    if (param.maxConstBufferCount)
        ConstBufferDX11::Init(param.maxConstBufferCount);
    if (param.maxTextureCount)
        TextureDX11::Init(param.maxTextureCount);

    stat_DIP = StatSet::AddStat("rhi'dip", "dip");
    stat_DP = StatSet::AddStat("rhi'dp", "dp");
    stat_DTL = StatSet::AddStat("rhi'dtl", "dtl");
    stat_DTS = StatSet::AddStat("rhi'dts", "dts");
    stat_DLL = StatSet::AddStat("rhi'dll", "dll");
    stat_SET_PS = StatSet::AddStat("rhi'set-ps", "set-ps");
    stat_SET_SS = StatSet::AddStat("rhi'set-ss", "set-ss");
    stat_SET_TEX = StatSet::AddStat("rhi'set-tex", "set-tex");
    stat_SET_CB = StatSet::AddStat("rhi'set-cb", "set-cb");
    stat_SET_VB = StatSet::AddStat("rhi'set-vb", "set-vb");
    stat_SET_IB = StatSet::AddStat("rhi'set-ib", "set-ib");
}

//==============================================================================
} // namespace rhi
