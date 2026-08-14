#pragma once

#include <d3d11.h>
#include <cstdint>

class cSpriteRenderer: public cRenderer
{
public:
    struct cSpriteVertexData
    {
        float x;
        float y;
        float z;
        uint32_t color;
        float u;
        float v;
        float mShaderParameters[4];
    };
private:

    struct cRenderState
    {
        int SpriteCount = 0, StateChangeCount = 0, TextureChangeCount = 0;
        ID3D11ShaderResourceView* Texture = nullptr;
        ID3D11PixelShader* PixelShader = nullptr;
        cSpriteRenderInfo::eBlendingMode LastBlendingMode = cSpriteRenderInfo::Invalid_Blend_Mode;
        int NumberOfBatchedVertices = 0;
        cSpriteVertexData* batchVertices = nullptr;
    };

    struct cShaderConstants
    {
        float TargetSize[2];
        float Padding[2];
    };

    bool mIsUnderDestruction=false;
    bool mIsInitDone=false;
    cPixieWindow &mBaseWindow;

    ID3D11Device *mDevice=nullptr;
    ID3D11DeviceContext *mDeviceContext=nullptr;
    ID3D11Buffer *mVertexBuffer=nullptr;
    ID3D11Buffer *mIndexBuffer=nullptr;
    ID3D11Buffer *mShaderConstants=nullptr;
    ID3D11SamplerState *mSamplerState=nullptr;
    ID3D11BlendState *mNormalBlendState=nullptr;
    ID3D11BlendState *mCopySourceBlendState=nullptr;
    ID3D11RasterizerState *mRasterizerState=nullptr;

    std::shared_ptr<cVertexShader> mDefaultVertexShader;
    std::shared_ptr<cPixelShader> mDefaultPixelShader;

    int mMaxSpritesPerFlush=0;
    bool mClearBeforeRender=false;
    bool mUseClipping = false;
    cRect mClippingRect;

    void UpdateBlending(cSpriteRenderInfo::eBlendingMode BlendingMode);
    void FlushBuffer(cSpriteVertexData*& batchVertices, int &NumberOfBatchedVertices, bool RelockBuffer);
    void Init();
    void renderSprites(cPixieWindow& window, cRenderState& renderState);
    void RenderSprites();
    void updateUsedTextures(cPixieWindow& window);
    void UpdateRenderTargetState();
    static void Rotate(cFloatPoint &Point, cFloatPoint Center, float s, float c);

protected:
    cDevice *mPixieDevice=nullptr;

public:
    cSpriteRenderer(cPixieWindow &BaseWindow): mBaseWindow(BaseWindow) {}
    ~cSpriteRenderer();
    void SetClearBeforeRender(bool ClearBeforeRender) { mClearBeforeRender=ClearBeforeRender; }
    virtual void Render() override;
};
