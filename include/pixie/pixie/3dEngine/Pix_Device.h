#pragma once

#include <d3d11.h>
#include <dxgi.h>

class cRenderer;
class cDeviceClearer;
class cTexture;

class cDevice
{
    ID3D11Device *mDevice = nullptr;
    ID3D11DeviceContext *mDeviceContext = nullptr;
    IDXGISwapChain *mSwapChain = nullptr;
    ID3D11Texture2D *mBackBufferTexture = nullptr;
    ID3D11RenderTargetView *mBackBufferSurface = nullptr;
    DXGI_SWAP_CHAIN_DESC mPresentParameters = {};
    volatile int mIsClosing = false;
    typedef std::list<cRenderer *> cRendererList;
    cRendererList mRenderers;
    cRenderer *mMainRenderer = nullptr;
    cDeviceClearer *mDeviceClearer = nullptr;
    bool mNeedClear = false;
    void Init();
    void Close();
    cRegisteredID mRenderingTimerID;
    void RenderingLoop();
    cDevice();
public:
    ~cDevice();
    static cDevice *Get();

    operator ID3D11Device *() const { return mDevice; }
    ID3D11Device *GetD3DObject() { return mDevice; }
    ID3D11DeviceContext *GetDeviceContext() { return mDeviceContext; }
    IDXGISwapChain *GetSwapChain() { return mSwapChain; }
    ID3D11RenderTargetView *GetBackBufferSurface() { return mBackBufferSurface; }

    void SetClearer(cDeviceClearer *pDeviceClearer);
    void AddRenderer(cRenderer *Renderer);
    void RemoveRenderer(cRenderer *Renderer);
    void AddMainRenderer(cRenderer *MainRenderer);
    const DXGI_SWAP_CHAIN_DESC &GetPresentParameters() const { return mPresentParameters; }

    int GetBackBufferWidth() const;
    int GetBackBufferHeight() const;
};

extern cDevice *theDevice;
