#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/2dEngine/sprite_renderer.h"
#include <d3dcompiler.h>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    const char *SpriteShaderSource = R"(
cbuffer SpriteConstants : register(b0)
{
    float2 TargetSize;
    float2 Padding;
};

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

struct VSInput
{
    float3 Position : POSITION;
    uint Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

float4 UnpackARGB(uint color)
{
    return float4(
        ((color >> 16) & 255) / 255.0f,
        ((color >> 8) & 255) / 255.0f,
        (color & 255) / 255.0f,
        ((color >> 24) & 255) / 255.0f);
}

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float2 clipPosition;
    clipPosition.x = input.Position.x * (2.0f / TargetSize.x) - 1.0f;
    clipPosition.y = 1.0f - input.Position.y * (2.0f / TargetSize.y);
    output.Position = float4(clipPosition, input.Position.z, 1.0f);
    output.Color = UnpackARGB(input.Color);
    output.TexCoord = input.TexCoord;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return SpriteTexture.Sample(SpriteSampler, input.TexCoord) * input.Color;
}
)";

    ID3DBlob *CompileShader(const char *entryPoint, const char *target)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob *shader = nullptr;
        ID3DBlob *errors = nullptr;
        HRESULT result = D3DCompile(
            SpriteShaderSource,
            strlen(SpriteShaderSource),
            "Pixie sprite shader",
            nullptr,
            nullptr,
            entryPoint,
            target,
            flags,
            0,
            &shader,
            &errors);

        if (FAILED(result))
        {
            std::string errorText = errors
                ? std::string(static_cast<const char *>(errors->GetBufferPointer()), errors->GetBufferSize())
                : "Unknown shader compilation error";
            if (errors)
                errors->Release();
            RELEASE_ASSERT_EXT(false, errorText);
        }

        if (errors)
            errors->Release();
        return shader;
    }
}

void cSpriteRenderer::Init()
{
    mMaxSpritesPerFlush = theGlobalConfig->get<int>("pixie_system.sprite_renderer.max_sprites_per_flush");
    ASSERT(mMaxSpritesPerFlush);

    mPixieDevice = cDevice::Get();
    mDevice = mPixieDevice->GetD3DObject();
    mDeviceContext = mPixieDevice->GetDeviceContext();

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = mMaxSpritesPerFlush * 4 * sizeof(cSpriteVertexData);
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3V(mDevice->CreateBuffer(&vertexBufferDesc, nullptr, &mVertexBuffer));

    std::vector<uint16_t> indices(static_cast<size_t>(mMaxSpritesPerFlush) * 6);
    for (int vertex = 0, index = 0; vertex < mMaxSpritesPerFlush * 4; vertex += 4, index += 6)
    {
        indices[index] = static_cast<uint16_t>(vertex);
        indices[index + 1] = static_cast<uint16_t>(vertex + 2);
        indices[index + 2] = static_cast<uint16_t>(vertex + 3);
        indices[index + 3] = static_cast<uint16_t>(vertex);
        indices[index + 4] = static_cast<uint16_t>(vertex + 1);
        indices[index + 5] = static_cast<uint16_t>(vertex + 2);
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = mMaxSpritesPerFlush * 6 * sizeof(uint16_t);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();
    D3V(mDevice->CreateBuffer(&indexBufferDesc, &indexData, &mIndexBuffer));

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = sizeof(cShaderConstants);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3V(mDevice->CreateBuffer(&constantBufferDesc, nullptr, &mShaderConstants));

    ID3DBlob *vertexShaderBlob = CompileShader("VSMain", "vs_5_0");
    ID3DBlob *pixelShaderBlob = CompileShader("PSMain", "ps_5_0");

    D3V(mDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &mVertexShader));
    D3V(mDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &mPixelShader));

    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(cSpriteVertexData, x), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(cSpriteVertexData, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(cSpriteVertexData, u), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    D3V(mDevice->CreateInputLayout(
        inputLayout,
        static_cast<UINT>(sizeof(inputLayout) / sizeof(inputLayout[0])),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &mInputLayout));

    vertexShaderBlob->Release();
    pixelShaderBlob->Release();

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    D3V(mDevice->CreateSamplerState(&samplerDesc, &mSamplerState));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3V(mDevice->CreateBlendState(&blendDesc, &mNormalBlendState));

    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    D3V(mDevice->CreateBlendState(&blendDesc, &mCopySourceBlendState));

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = TRUE;
    D3V(mDevice->CreateRasterizerState(&rasterizerDesc, &mRasterizerState));

    mIsInitDone = true;
}

void cSpriteRenderer::UpdateBlending(cSpriteRenderInfo::eBlendingMode BlendingMode)
{
    ID3D11BlendState *blendState = nullptr;
    switch (BlendingMode)
    {
    case cSpriteRenderInfo::Blend_Normal:
        blendState = mNormalBlendState;
        break;
    case cSpriteRenderInfo::Blend_CopySource:
        blendState = mCopySourceBlendState;
        break;
    case cSpriteRenderInfo::Invalid_Blend_Mode:
        ASSERT(false);
        return;
    }

    const float blendFactor[4] = { 0, 0, 0, 0 };
    mDeviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
}

cSpriteRenderer::~cSpriteRenderer()
{
    mIsUnderDestruction = true;

    if (mRasterizerState) mRasterizerState->Release();
    if (mCopySourceBlendState) mCopySourceBlendState->Release();
    if (mNormalBlendState) mNormalBlendState->Release();
    if (mSamplerState) mSamplerState->Release();
    if (mInputLayout) mInputLayout->Release();
    if (mPixelShader) mPixelShader->Release();
    if (mVertexShader) mVertexShader->Release();
    if (mShaderConstants) mShaderConstants->Release();
    if (mIndexBuffer) mIndexBuffer->Release();
    if (mVertexBuffer) mVertexBuffer->Release();
}

void cSpriteRenderer::Rotate(cFloatPoint &Point, cFloatPoint Center, float s, float c)
{
    Point -= Center;
    cFloatPoint Rotated(Point.x * c - Point.y * s, Point.x * s + Point.y * c);
    Point = Rotated;
    Point += Center;
}

void cSpriteRenderer::renderSprites(cPixieWindow& window, cRenderState& renderState)
{
    const auto [useClipping, clippingRect] = window.getSpriteClipping();
    if (useClipping != mUseClipping || (useClipping && clippingRect != mClippingRect))
    {
        FlushBuffer(renderState.batchVertices, renderState.NumberOfBatchedVertices, true);
        mUseClipping = useClipping;
        mClippingRect = clippingRect;

        D3D11_RECT rect = {};
        if (mUseClipping)
        {
            rect.left = clippingRect.left();
            rect.right = clippingRect.right();
            rect.top = clippingRect.top();
            rect.bottom = clippingRect.bottom();
        }
        else
        {
            rect.left = 0;
            rect.top = 0;
            rect.right = mRenderSurfaceWidth;
            rect.bottom = mRenderSurfaceHeight;
        }
        mDeviceContext->RSSetScissorRects(1, &rect);
    }

    for (auto& sprite : window.mSprites)
    {
        const cSpriteRenderInfo RenderInfo = sprite->GetRenderInfo();
        auto& batchVertices = renderState.batchVertices;
        auto& NumberOfBatchedVertices = renderState.NumberOfBatchedVertices;
        float Z = 0.5f;

        if (RenderInfo.mTexture)
        {
            if (RenderInfo.mBlendingMode != renderState.LastBlendingMode)
            {
                ++renderState.StateChangeCount;
                FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
                renderState.LastBlendingMode = RenderInfo.mBlendingMode;
                UpdateBlending(renderState.LastBlendingMode);
            }

            if (RenderInfo.mTexture->mShaderResourceView != renderState.Texture)
            {
                ++renderState.TextureChangeCount;
                FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
                renderState.Texture = RenderInfo.mTexture->mShaderResourceView;
                mDeviceContext->PSSetShaderResources(0, 1, &renderState.Texture);
            }

            ++renderState.SpriteCount;
            cFloatPoint TopLeft(RenderInfo.mRect.topLeft());
            cFloatPoint TopRight(RenderInfo.mRect.topRight());
            cFloatPoint BottomLeft(RenderInfo.mRect.bottomLeft());
            cFloatPoint BottomRight(RenderInfo.mRect.bottomRight());

            // D3D11 pixel centers differ from D3D9. Keep the old inclusive cRect
            // semantics, but remove the D3D9 -0.5 half-pixel correction.
            TopRight += cFloatPoint(1.0f, 0.0f);
            BottomLeft += cFloatPoint(0.0f, 1.0f);
            BottomRight += cFloatPoint(1.0f, 1.0f);

            if (RenderInfo.mRotation)
            {
                cFloatPoint Center(RenderInfo.mRect.center());
                float Rad = RenderInfo.mRotation * 3.14159265358979323846f / 180.0f;
                float s = sin(Rad);
                float c = cos(Rad);
                Rotate(TopLeft, Center, s, c);
                Rotate(TopRight, Center, s, c);
                Rotate(BottomLeft, Center, s, c);
                Rotate(BottomRight, Center, s, c);
            }

            batchVertices[NumberOfBatchedVertices].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::TopLeft].GetARGBColor();
            batchVertices[NumberOfBatchedVertices].x = TopLeft.x;
            batchVertices[NumberOfBatchedVertices].y = TopLeft.y;
            batchVertices[NumberOfBatchedVertices].z = Z;
            batchVertices[NumberOfBatchedVertices].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
            batchVertices[NumberOfBatchedVertices].v = RenderInfo.mTexture->GetTextureInfo().mTop;

            batchVertices[NumberOfBatchedVertices + 1].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::TopRight].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 1].x = TopRight.x;
            batchVertices[NumberOfBatchedVertices + 1].y = TopRight.y;
            batchVertices[NumberOfBatchedVertices + 1].z = Z;
            batchVertices[NumberOfBatchedVertices + 1].u = RenderInfo.mTexture->GetTextureInfo().mRight;
            batchVertices[NumberOfBatchedVertices + 1].v = RenderInfo.mTexture->GetTextureInfo().mTop;

            batchVertices[NumberOfBatchedVertices + 2].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::BottomRight].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 2].x = BottomRight.x;
            batchVertices[NumberOfBatchedVertices + 2].y = BottomRight.y;
            batchVertices[NumberOfBatchedVertices + 2].z = Z;
            batchVertices[NumberOfBatchedVertices + 2].u = RenderInfo.mTexture->GetTextureInfo().mRight;
            batchVertices[NumberOfBatchedVertices + 2].v = RenderInfo.mTexture->GetTextureInfo().mBottom;

            batchVertices[NumberOfBatchedVertices + 3].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::BottomLeft].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 3].x = BottomLeft.x;
            batchVertices[NumberOfBatchedVertices + 3].y = BottomLeft.y;
            batchVertices[NumberOfBatchedVertices + 3].z = Z;
            batchVertices[NumberOfBatchedVertices + 3].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
            batchVertices[NumberOfBatchedVertices + 3].v = RenderInfo.mTexture->GetTextureInfo().mBottom;

            NumberOfBatchedVertices += 4;
        }

        if (NumberOfBatchedVertices > (mMaxSpritesPerFlush - 1) * 4)
            FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
    }

    for (auto& subWindow : window.mSubWindows | std::views::reverse)
        renderSprites(*subWindow, renderState);
}

void cSpriteRenderer::RenderSprites()
{
    cRenderState renderState;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    D3V(mDeviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    renderState.batchVertices = static_cast<cSpriteVertexData *>(mapped.pData);

    UINT stride = sizeof(cSpriteVertexData);
    UINT offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    renderSprites(mBaseWindow, renderState);
    FlushBuffer(renderState.batchVertices, renderState.NumberOfBatchedVertices, false);
}

void cSpriteRenderer::FlushBuffer(cSpriteVertexData*& batchVertices, int &NumberOfBatchedVertices, bool RelockBuffer)
{
    mDeviceContext->Unmap(mVertexBuffer, 0);
    batchVertices = nullptr;

    if (NumberOfBatchedVertices)
    {
        mDeviceContext->DrawIndexed(NumberOfBatchedVertices / 4 * 6, 0, 0);
        NumberOfBatchedVertices = 0;
    }

    if (RelockBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        D3V(mDeviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        batchVertices = static_cast<cSpriteVertexData *>(mapped.pData);
    }
}

void cSpriteRenderer::updateUsedTextures(cPixieWindow& window)
{
    for (auto& sprite : window.mSprites)
    {
        cSpriteRenderInfo RenderInfo = sprite->GetRenderInfo();
        if (RenderInfo.mTexture && RenderInfo.mTexture->DoesNeedUpdateBeforeUse())
            const_cast<cTexture*>(RenderInfo.mTexture)->Update();
    }

    for (auto& subWindow : window.mSubWindows)
        updateUsedTextures(*subWindow);
}

void cSpriteRenderer::UpdateRenderTargetState()
{
    ASSERT(mRenderSurface);
    ASSERT(mRenderSurfaceWidth > 0 && mRenderSurfaceHeight > 0);

    ID3D11ShaderResourceView *nullView = nullptr;
    mDeviceContext->PSSetShaderResources(0, 1, &nullView);
    mDeviceContext->OMSetRenderTargets(1, &mRenderSurface, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(mRenderSurfaceWidth);
    viewport.Height = static_cast<float>(mRenderSurfaceHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    mDeviceContext->RSSetViewports(1, &viewport);

    D3D11_RECT fullRect = { 0, 0, mRenderSurfaceWidth, mRenderSurfaceHeight };
    mDeviceContext->RSSetScissorRects(1, &fullRect);
    mUseClipping = false;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    D3V(mDeviceContext->Map(mShaderConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    auto constants = static_cast<cShaderConstants *>(mapped.pData);
    constants->TargetSize[0] = static_cast<float>(mRenderSurfaceWidth);
    constants->TargetSize[1] = static_cast<float>(mRenderSurfaceHeight);
    constants->Padding[0] = 0;
    constants->Padding[1] = 0;
    mDeviceContext->Unmap(mShaderConstants, 0);
}

void cSpriteRenderer::Render()
{
    if (!mIsInitDone)
        Init();

    updateUsedTextures(mBaseWindow);
    UpdateRenderTargetState();

    if (mClearBeforeRender)
    {
        const float clearColor[4] = { 0, 0, 0, 0 };
        mDeviceContext->ClearRenderTargetView(mRenderSurface, clearColor);
    }

    mDeviceContext->IASetInputLayout(mInputLayout);
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
    mDeviceContext->VSSetConstantBuffers(0, 1, &mShaderConstants);
    mDeviceContext->PSSetShader(mPixelShader, nullptr, 0);
    mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
    mDeviceContext->RSSetState(mRasterizerState);

    mBaseWindow.CheckOwnerlessSprites();
    RenderSprites();
}
