#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/2dEngine/sprite_renderer.h"

void cSpriteRenderer::Init()
{
    mDefaultVertexShader = theShaderManager->vertexShader("default");
    mDefaultPixelShader = theShaderManager->pixelShader("default");

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

            ID3D11PixelShader* pixelShader = RenderInfo.mShader
                ? RenderInfo.mShader->shader()
                : mDefaultPixelShader->shader();
            if (pixelShader != renderState.PixelShader)
            {
                ++renderState.StateChangeCount;
                FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
                renderState.PixelShader = pixelShader;
                mDeviceContext->PSSetShader(renderState.PixelShader, nullptr, 0);
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

            float width = static_cast<float>(RenderInfo.mRect.width());
            float height = static_cast<float>(RenderInfo.mRect.height());

            batchVertices[NumberOfBatchedVertices].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::TopLeft].GetARGBColor();
            batchVertices[NumberOfBatchedVertices].x = TopLeft.x;
            batchVertices[NumberOfBatchedVertices].y = TopLeft.y;
            batchVertices[NumberOfBatchedVertices].z = Z;
            batchVertices[NumberOfBatchedVertices].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
            batchVertices[NumberOfBatchedVertices].v = RenderInfo.mTexture->GetTextureInfo().mTop;
            batchVertices[NumberOfBatchedVertices].edgeDistances[0] = 0.0f; // Top edge distance
            batchVertices[NumberOfBatchedVertices].edgeDistances[1] = 0.0f; // Right edge distance
            batchVertices[NumberOfBatchedVertices].edgeDistances[2] = width; // Bottom edge distance
            batchVertices[NumberOfBatchedVertices].edgeDistances[3] = height; // Left edge distance 
            std::copy(std::begin(RenderInfo.mShaderParameters), std::end(RenderInfo.mShaderParameters),
                batchVertices[NumberOfBatchedVertices].mShaderParameters);

            batchVertices[NumberOfBatchedVertices + 1].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::TopRight].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 1].x = TopRight.x;
            batchVertices[NumberOfBatchedVertices + 1].y = TopRight.y;
            batchVertices[NumberOfBatchedVertices + 1].z = Z;
            batchVertices[NumberOfBatchedVertices + 1].u = RenderInfo.mTexture->GetTextureInfo().mRight;
            batchVertices[NumberOfBatchedVertices + 1].v = RenderInfo.mTexture->GetTextureInfo().mTop;
            batchVertices[NumberOfBatchedVertices + 1].edgeDistances[0] = width; // Top edge distance
            batchVertices[NumberOfBatchedVertices + 1].edgeDistances[1] = 0.0f; // Right edge distance
            batchVertices[NumberOfBatchedVertices + 1].edgeDistances[2] = 0.0f; // Bottom edge distance
            batchVertices[NumberOfBatchedVertices + 1].edgeDistances[3] = height; // Left edge distance
            std::copy(std::begin(RenderInfo.mShaderParameters), std::end(RenderInfo.mShaderParameters),
                batchVertices[NumberOfBatchedVertices + 1].mShaderParameters);

            batchVertices[NumberOfBatchedVertices + 2].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::BottomRight].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 2].x = BottomRight.x;
            batchVertices[NumberOfBatchedVertices + 2].y = BottomRight.y;
            batchVertices[NumberOfBatchedVertices + 2].z = Z;
            batchVertices[NumberOfBatchedVertices + 2].u = RenderInfo.mTexture->GetTextureInfo().mRight;
            batchVertices[NumberOfBatchedVertices + 2].v = RenderInfo.mTexture->GetTextureInfo().mBottom;
            batchVertices[NumberOfBatchedVertices + 2].edgeDistances[0] = width; // Top edge distance
            batchVertices[NumberOfBatchedVertices + 2].edgeDistances[1] = height; // Right edge distance
            batchVertices[NumberOfBatchedVertices + 2].edgeDistances[2] = 0.0f; // Bottom edge distance
            batchVertices[NumberOfBatchedVertices + 2].edgeDistances[3] = 0.0f; // Left edge distance
            std::copy(std::begin(RenderInfo.mShaderParameters), std::end(RenderInfo.mShaderParameters),
                batchVertices[NumberOfBatchedVertices + 2].mShaderParameters);

            batchVertices[NumberOfBatchedVertices + 3].color = RenderInfo.mCornerColors[cSpriteColor::CornerPosition::BottomLeft].GetARGBColor();
            batchVertices[NumberOfBatchedVertices + 3].x = BottomLeft.x;
            batchVertices[NumberOfBatchedVertices + 3].y = BottomLeft.y;
            batchVertices[NumberOfBatchedVertices + 3].z = Z;
            batchVertices[NumberOfBatchedVertices + 3].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
            batchVertices[NumberOfBatchedVertices + 3].v = RenderInfo.mTexture->GetTextureInfo().mBottom;
            batchVertices[NumberOfBatchedVertices + 3].edgeDistances[0] = 0.0f; // Top edge distance
            batchVertices[NumberOfBatchedVertices + 3].edgeDistances[1] = height; // Right edge distance
            batchVertices[NumberOfBatchedVertices + 3].edgeDistances[2] = width; // Bottom edge distance
            batchVertices[NumberOfBatchedVertices + 3].edgeDistances[3] = 0.0f; // Left edge distance
            std::copy(std::begin(RenderInfo.mShaderParameters), std::end(RenderInfo.mShaderParameters),
                batchVertices[NumberOfBatchedVertices + 3].mShaderParameters);

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
    renderState.PixelShader = mDefaultPixelShader->shader();

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    D3V(mDeviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    renderState.batchVertices = static_cast<cSpriteVertexData *>(mapped.pData);

    UINT stride = sizeof(cSpriteVertexData);
    UINT offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Bind global pixel-shader constant buffers.
    for (const auto& provider : theShaderConstantProviders)
    {
        ID3D11Buffer* buffer = provider->shaderConstantBuffer();

        mDeviceContext->PSSetConstantBuffers(
            provider->slot(),
            1,
            &buffer);
    }

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

    mDeviceContext->IASetInputLayout(mDefaultVertexShader->inputLayout());
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mDeviceContext->VSSetShader(mDefaultVertexShader->shader(), nullptr, 0);
    mDeviceContext->VSSetConstantBuffers(0, 1, &mShaderConstants);
    mDeviceContext->PSSetShader(mDefaultPixelShader->shader(), nullptr, 0);
    mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);
    mDeviceContext->RSSetState(mRasterizerState);

    mBaseWindow.CheckOwnerlessSprites();
    RenderSprites();
}
