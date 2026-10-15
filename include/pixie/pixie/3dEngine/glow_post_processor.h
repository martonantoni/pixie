#pragma once
#include <d3d11.h>
#include <memory>

class cPixelShader;
enum class eDisplayMode;

// Fullscreen postprocessing for the desktop only; offscreen root windows stay unchanged.
class cGlowPostProcessor
{
    ID3D11Device* mDevice = nullptr;
    ID3D11DeviceContext* mContext = nullptr;
    ID3D11Texture2D* mColor = nullptr;
    ID3D11RenderTargetView* mColorRTV = nullptr;
    ID3D11ShaderResourceView* mColorSRV = nullptr;
    ID3D11Texture2D* mBlur[2] = {};
    ID3D11RenderTargetView* mBlurRTV[2] = {};
    ID3D11ShaderResourceView* mBlurSRV[2] = {};
    ID3D11VertexShader* mFullscreenVS = nullptr;
    ID3D11SamplerState* mLinearSampler = nullptr;
    ID3D11BlendState* mOpaqueBlend = nullptr;
    ID3D11RasterizerState* mRasterizer = nullptr;
    ID3D11Buffer* mConstants = nullptr;
    std::shared_ptr<cPixelShader> mBlurShader;
    std::shared_ptr<cPixelShader> mPresentShader;
    int mWidth = 0, mHeight = 0;
    void init();
    void draw(ID3D11RenderTargetView* output, int width, int height,
              ID3D11PixelShader* shader, ID3D11ShaderResourceView* source0,
              ID3D11ShaderResourceView* source1, float dx, float dy,
              float mode, float exposure);
public:
    ~cGlowPostProcessor();
    void ensureTargets(int width, int height);
    void releaseTargets();
    ID3D11RenderTargetView* colorRenderTarget() const { return mColorRTV; }
    ID3D11ShaderResourceView* colorShaderResourceView() const { return mColorSRV; }
    void present(ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* emission,
                 ID3D11RenderTargetView* destination, eDisplayMode mode, int width, int height);
};
