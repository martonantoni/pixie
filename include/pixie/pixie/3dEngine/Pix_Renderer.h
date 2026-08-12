#pragma once

#include <d3d11.h>

class cRenderer abstract
{
protected:
    ID3D11RenderTargetView *mRenderSurface = nullptr;
    int mRenderSurfaceWidth = 0;
    int mRenderSurfaceHeight = 0;
public:
    virtual ~cRenderer() {}
    virtual void Render() abstract;

    void SetRenderSurface(ID3D11RenderTargetView *RenderSurface)
    {
        mRenderSurface = RenderSurface;
        mRenderSurfaceWidth = 0;
        mRenderSurfaceHeight = 0;

        if (!mRenderSurface)
            return;

        ID3D11Resource *resource = nullptr;
        mRenderSurface->GetResource(&resource);
        if (!resource)
            return;

        ID3D11Texture2D *texture = nullptr;
        if (SUCCEEDED(resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&texture))))
        {
            D3D11_TEXTURE2D_DESC desc = {};
            texture->GetDesc(&desc);
            mRenderSurfaceWidth = static_cast<int>(desc.Width);
            mRenderSurfaceHeight = static_cast<int>(desc.Height);
            texture->Release();
        }
        resource->Release();
    }
};
