#pragma once

#include <d3d11.h>
#include <cstdint>
#include <vector>

struct cTextureFile;
class cTextureManager;
class cSpriteRenderer;

struct cTextureInfo
{
    float mTop,mLeft,mBottom,mRight;
    cRect mRect;
    BOOL mIsWholeSurface=false;
    cTextureInfo(const cRect &Rect, cPoint SurfaceSize);
    cTextureInfo(cPoint SurfaceSize);
};

class cTexture: public cIntrusiveRefCount
{
    friend cTextureManager;
    friend cSpriteRenderer;

    ID3D11Texture2D *mTexture = nullptr;
    ID3D11ShaderResourceView *mShaderResourceView = nullptr;
    int mSurfaceWidth,mSurfaceHeight;
    cTextureInfo mTextureInfo;

    bool mIsWritable = false;
    bool mWritableLockIsReadOnly = false;
    std::vector<uint8_t> mWritablePixels;

protected:
    bool mNeedUpdateBeforeUse=false;
    ID3D11RenderTargetView *mSurface = nullptr; // render-target view for renderable textures
    cTexture(const cTextureInfo &TextureInfo): mTextureInfo(TextureInfo) {}
    ~cTexture();
    void InitForRenderTarget(cPoint Size);
    void InitForWritable(cPoint Size);

public:
    cTexture(const cTexture &BaseTexture,const cTextureInfo &TextureInfo);
    cTexture(ID3D11Texture2D *BaseTexture,int TextureWidth,int TextureHeight);

// functions for creating new Texture objects:
    static tIntrusivePtr<cTexture> CreateRenderTarget(unsigned int Width, unsigned int Height) { return CreateRenderTarget(cPoint(Width, Height)); }
    static tIntrusivePtr<cTexture> CreateRenderTarget(cPoint Size);
    static tIntrusivePtr<cTexture> CreateWriteable(cPoint Size);
    tIntrusivePtr<cTexture> CreateSubTexture(const cRect &SubRect) const;
    static const unsigned int Flip_Horizontal = 1;
    static const unsigned int Flip_Vertical = 2;
    tIntrusivePtr<cTexture> CreateFlipped(unsigned int FlipFlags) const;

// Property querying:
    const cTextureInfo &GetTextureInfo() const { return mTextureInfo; }
    int GetSurfaceWidth() const { return mSurfaceWidth; }
    int GetSurfaceHeight() const { return mSurfaceHeight; }
    int GetTextureWidth() const { return mTextureInfo.mRect.width(); }
    int GetTextureHeight() const { return mTextureInfo.mRect.height(); }
    cRect GetTextureRect() const { return mTextureInfo.mRect; }
    cPoint GetSize() const { return { GetTextureWidth(),GetTextureHeight() }; }

// functions reserved for Textures created with CreateRenderTarget:
    void SetAsRenderTarget();
    bool DoesNeedUpdateBeforeUse() const { return mNeedUpdateBeforeUse; }
    virtual void Update() {}
    void Clear(uint32_t Color);

// functions reserved for Textures created with CreateWritable:
    struct cLockInfo
    {
        int mPitch;
        char *mBytes=nullptr;
    };
    enum class IsReadOnly { yes, no };
    cLockInfo LockSurface(IsReadOnly isReadOnly);
    void UnlockSurface();
};
