#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

cDevice *theDevice = NULL;

cDevice::cDevice()
{
    mIsClosing = false;
    theDevice = this;
}

cDevice::~cDevice()
{
    if (mDeviceContext)
        mDeviceContext->ClearState();

    if (mBackBufferSurface)
        mBackBufferSurface->Release();
    if (mBackBufferTexture)
        mBackBufferTexture->Release();
    if (mSwapChain)
        mSwapChain->Release();
    if (mDeviceContext)
        mDeviceContext->Release();
    if (mDevice)
        mDevice->Release();

    theDevice = NULL;
}

void cDevice::Init()
{
    ZeroMemory(&mPresentParameters, sizeof(mPresentParameters));

    cPrimaryWindow &PrimaryWindow = cPrimaryWindow::Get();

    mPresentParameters.BufferCount = 1;
    mPresentParameters.BufferDesc.Width = 0;
    mPresentParameters.BufferDesc.Height = 0;
    mPresentParameters.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    mPresentParameters.BufferDesc.RefreshRate.Numerator = 0;
    mPresentParameters.BufferDesc.RefreshRate.Denominator = 1;
    mPresentParameters.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    mPresentParameters.OutputWindow = PrimaryWindow.mWindowHandle;
    mPresentParameters.SampleDesc.Count = 1;
    mPresentParameters.SampleDesc.Quality = 0;
    mPresentParameters.Windowed = TRUE;
    mPresentParameters.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    mPresentParameters.Flags = 0;

    UINT deviceFlags = 0;
#ifdef _DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL requestedFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL createdFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        requestedFeatureLevels,
        static_cast<UINT>(sizeof(requestedFeatureLevels) / sizeof(requestedFeatureLevels[0])),
        D3D11_SDK_VERSION,
        &mPresentParameters,
        &mSwapChain,
        &mDevice,
        &createdFeatureLevel,
        &mDeviceContext);

#ifdef _DEBUG
    // The debug layer is optional on modern Windows installations.
    if (result == DXGI_ERROR_SDK_COMPONENT_MISSING)
    {
        deviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            requestedFeatureLevels,
            static_cast<UINT>(sizeof(requestedFeatureLevels) / sizeof(requestedFeatureLevels[0])),
            D3D11_SDK_VERSION,
            &mPresentParameters,
            &mSwapChain,
            &mDevice,
            &createdFeatureLevel,
            &mDeviceContext);
    }
#endif

    StopOnError(result);

    D3V(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&mBackBufferTexture)));
    D3V(mDevice->CreateRenderTargetView(mBackBufferTexture, nullptr, &mBackBufferSurface));
    mDeviceContext->OMSetRenderTargets(1, &mBackBufferSurface, nullptr);

    mRenderingTimerID = theMainThread->AddTimer([this]() { RenderingLoop(); }, cTimerRequest(10));
    mNeedClear = theGlobalConfig->get<bool>("pixie_system.clear_device_before_frame", false);
}

void cDevice::Close()
{
    mIsClosing = true;
}

void cDevice::RenderingLoop()
{
    if (mIsClosing)
        return;

    theLogicServer.Tick();

    theRenderers.Call();

    if (mNeedClear && mDeviceClearer)
        mDeviceClearer->ClearDevice(mDeviceContext);

    for (cRendererList::iterator i = mRenderers.begin(), iend = mRenderers.end(); i != iend; ++i)
        (*i)->Render();

    if (mMainRenderer)
        mMainRenderer->Render();

    for (auto& window : thePixieDesktop.ownerlessWindows())
        window->CheckOwnerlessSprites();

    StopOnError(mSwapChain->Present(1, 0));
}

cDevice *cDevice::Get()
{
    static cDevice *Instance = NULL;
    if (!Instance)
    {
        Instance = new cDevice;
        theMainThread->callback([]() { Instance->Init(); }, eCallbackType::Wait);
    }
    return Instance;
}

void cDevice::SetClearer(cDeviceClearer *pDeviceClearer)
{
    mDeviceClearer = pDeviceClearer;
}

void cDevice::AddRenderer(cRenderer *Renderer)
{
    mRenderers.push_back(Renderer);
}

void cDevice::RemoveRenderer(cRenderer *Renderer)
{
    if (mMainRenderer == Renderer)
    {
        mMainRenderer = nullptr;
        return;
    }
    mRenderers.remove(Renderer);
}

void cDevice::AddMainRenderer(cRenderer *MainRenderer)
{
    mMainRenderer = MainRenderer;
    mMainRenderer->SetRenderSurface(mBackBufferSurface);
}

int cDevice::GetBackBufferWidth() const
{
    D3D11_TEXTURE2D_DESC desc;
    mBackBufferTexture->GetDesc(&desc);
    return static_cast<int>(desc.Width);
}

int cDevice::GetBackBufferHeight() const
{
    D3D11_TEXTURE2D_DESC desc;
    mBackBufferTexture->GetDesc(&desc);
    return static_cast<int>(desc.Height);
}