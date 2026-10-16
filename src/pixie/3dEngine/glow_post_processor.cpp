#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/Desktop2.h"
#include <d3dcompiler.h>

// Code by ChatGPT

namespace
{
    template<typename T> void release(T*& pointer)
    {
        if (pointer) { pointer->Release(); pointer = nullptr; }
    }
    struct alignas(16) cPostConstants
    {
        float texelSize[2];
        float displayMode;
        float exposure;
    };
    void makeTexture(ID3D11Device* device, int width, int height, DXGI_FORMAT textureFormat,
        ID3D11Texture2D** texture, ID3D11RenderTargetView** rtv,
        ID3D11ShaderResourceView** srv)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = static_cast<UINT>(width);
        desc.Height = static_cast<UINT>(height);
        desc.ArraySize = desc.MipLevels = 1;
        desc.Format = textureFormat;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        D3V(device->CreateTexture2D(&desc, nullptr, texture));
        D3V(device->CreateRenderTargetView(*texture, nullptr, rtv));
        D3V(device->CreateShaderResourceView(*texture, nullptr, srv));
    }
}

cGlowPostProcessor::~cGlowPostProcessor()
{
    // Ensure no stale framebuffer bindings outlive our resources.
    if (mContext)
    {
        mContext->OMSetRenderTargets(0, nullptr, nullptr);
        ID3D11ShaderResourceView* nullViews[3] = {};
        mContext->PSSetShaderResources(0, 3, nullViews);
    }
    releaseTargets();
    release(mConstants);
    release(mRasterizer);
    release(mOpaqueBlend);
    release(mLinearSampler);
    release(mFullscreenVS);
}

void cGlowPostProcessor::init()
{
    if (mDevice) return;
    mDevice = theDevice->GetD3DObject();
    mContext = theDevice->GetDeviceContext();
    mBlurShader = theShaderManager->pixelShader("glow_blur");
    mPresentShader = theShaderManager->pixelShader("glow_present");

    // SV_VertexID fullscreen triangle; no input layout or vertex buffer needed.
    constexpr char vertexSource[] = R"(
        struct Output { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
        Output VSMain(uint id : SV_VertexID)
        {
            Output o;
            o.uv = float2((id << 1) & 2, id & 2);
            o.position = float4(o.uv * float2(2, -2) + float2(-1, 1), 0, 1);
            return o;
        }
    )";
    ID3DBlob* blob = nullptr;
    ID3DBlob* errors = nullptr;
    HRESULT hr = D3DCompile(vertexSource, sizeof(vertexSource) - 1,
        "glow_fullscreen", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &blob, &errors);
    if (errors) errors->Release();
    D3V(hr);
    D3V(mDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mFullscreenVS));
    blob->Release();

    D3D11_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD = D3D11_FLOAT32_MAX;
    D3V(mDevice->CreateSamplerState(&sampler, &mLinearSampler));

    D3D11_BLEND_DESC blend = {};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3V(mDevice->CreateBlendState(&blend, &mOpaqueBlend));

    D3D11_RASTERIZER_DESC raster = {};
    raster.FillMode = D3D11_FILL_SOLID;
    raster.CullMode = D3D11_CULL_NONE;
    raster.DepthClipEnable = TRUE;
    D3V(mDevice->CreateRasterizerState(&raster, &mRasterizer));

    D3D11_BUFFER_DESC constantDesc = {};
    constantDesc.ByteWidth = sizeof(cPostConstants);
    constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3V(mDevice->CreateBuffer(&constantDesc, nullptr, &mConstants));
}

void cGlowPostProcessor::releaseTargets()
{
    if (mContext)
    {
        mContext->OMSetRenderTargets(0, nullptr, nullptr);
        ID3D11ShaderResourceView* nullViews[3] = {};
        mContext->PSSetShaderResources(0, 3, nullViews);
    }
    release(mColorSRV); release(mColorRTV); release(mColor);
    for (int i = 0; i < 2; ++i)
    {
        release(mBlurSRV[i]); release(mBlurRTV[i]); release(mBlur[i]);
        release(mWideBlurSRV[i]); release(mWideBlurRTV[i]); release(mWideBlur[i]);
    }
    mWidth = mHeight = 0;
}

void cGlowPostProcessor::ensureTargets(int width, int height)
{
    init();
    if (mColor && width == mWidth && height == mHeight)
    {
        return;
    }
    releaseTargets();
    mWidth = width;
    mHeight = height;
    // RT0 matches the swap-chain's BGRA8 format, preserving its color range.
    makeTexture(mDevice, width, height, DXGI_FORMAT_B8G8R8A8_UNORM,
        &mColor, &mColorRTV, &mColorSRV);
    const int halfWidth = (width + 1) / 2;
    const int halfHeight = (height + 1) / 2;
    const int wideWidth = (width + 7) / 8;
    const int wideHeight = (height + 7) / 8;
    for (int i = 0; i < 2; ++i)
    {
        makeTexture(mDevice, halfWidth, halfHeight, DXGI_FORMAT_R16G16B16A16_FLOAT,
            &mBlur[i], &mBlurRTV[i], &mBlurSRV[i]);
        makeTexture(mDevice, wideWidth, wideHeight, DXGI_FORMAT_R16G16B16A16_FLOAT,
            &mWideBlur[i], &mWideBlurRTV[i], &mWideBlurSRV[i]);
    }
}

void cGlowPostProcessor::draw(ID3D11RenderTargetView* output, int width, int height,
    ID3D11PixelShader* shader, ID3D11ShaderResourceView* source0,
    ID3D11ShaderResourceView* source1, ID3D11ShaderResourceView* source2,
    float dx, float dy, float mode, float exposure)
{
    // Remove SRV hazards before changing render targets.
    ID3D11ShaderResourceView* nullViews[3] = {};
    mContext->PSSetShaderResources(0, 3, nullViews);
    mContext->OMSetRenderTargets(1, &output, nullptr);
    D3D11_VIEWPORT vp = {};
    vp.Width = float(width); vp.Height = float(height); vp.MaxDepth = 1.0f;
    mContext->RSSetViewports(1, &vp);
    D3D11_RECT rect = { 0, 0, width, height };
    mContext->RSSetScissorRects(1, &rect);
    mContext->RSSetState(mRasterizer);
    mContext->OMSetBlendState(mOpaqueBlend, nullptr, 0xffffffff);
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    D3V(mContext->Map(mConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    *static_cast<cPostConstants*>(mapped.pData) = {{dx, dy}, mode, exposure};
    mContext->Unmap(mConstants, 0);
    mContext->PSSetConstantBuffers(0, 1, &mConstants);
    mContext->PSSetSamplers(0, 1, &mLinearSampler);
    ID3D11ShaderResourceView* sources[3] = { source0, source1, source2 };
    mContext->PSSetShaderResources(0, 3, sources);
    mContext->IASetInputLayout(nullptr);
    mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11Buffer* noBuffer = nullptr;
    UINT stride = 0, offset = 0;
    mContext->IASetVertexBuffers(0, 1, &noBuffer, &stride, &offset);
    mContext->VSSetShader(mFullscreenVS, nullptr, 0);
    mContext->PSSetShader(shader, nullptr, 0);
    mContext->Draw(3, 0);
    mContext->PSSetShaderResources(0, 3, nullViews);
}

void cGlowPostProcessor::present(ID3D11ShaderResourceView* color,
    ID3D11ShaderResourceView* emission, ID3D11RenderTargetView* destination,
    eDisplayMode mode, int width, int height)
{
    init();
    if (!mPresentShader || !mPresentShader->shader())
    {
        return;
    }
    ID3D11ShaderResourceView* tightGlow = emission;
    ID3D11ShaderResourceView* wideGlow = nullptr;

    if (mode == eDisplayMode::Glow && mBlurShader && mBlurShader->shader() && emission)
    {
        const int halfWidth = (width + 1) / 2;
        const int halfHeight = (height + 1) / 2;
        const int wideWidth = (width + 7) / 8;
        const int wideHeight = (height + 7) / 8;

        // Tight halo: half-resolution horizontal and vertical passes.
        draw(mBlurRTV[0], halfWidth, halfHeight, mBlurShader->shader(),
             emission, nullptr, nullptr, 1.0f / halfWidth, 0, 0, 1);
        draw(mBlurRTV[1], halfWidth, halfHeight, mBlurShader->shader(),
             mBlurSRV[0], nullptr, nullptr, 0, 1.0f / halfHeight, 0, 1);
        tightGlow = mBlurSRV[1];

        // Wide halo: blur the already-filtered half-resolution result at 1/8 resolution.
        // This prefiltering avoids skipping narrow, bright glyphs when downsampling.
        draw(mWideBlurRTV[0], wideWidth, wideHeight, mBlurShader->shader(),
             tightGlow, nullptr, nullptr, 1.0f / wideWidth, 0, 0, 1);
        draw(mWideBlurRTV[1], wideWidth, wideHeight, mBlurShader->shader(),
             mWideBlurSRV[0], nullptr, nullptr, 0, 1.0f / wideHeight, 0, 1);
        wideGlow = mWideBlurSRV[1];
    }

    draw(destination, width, height, mPresentShader->shader(),
         color, mode == eDisplayMode::Emission ? emission : tightGlow, wideGlow,
         0, 0, static_cast<float>(mode), 1.0f);
    // The postprocess intentionally leaves the back buffer bound at frame end.
}
