#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

namespace
{
    constexpr DXGI_FORMAT TextureFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

    void ArgbToFloat4(uint32_t color, float result[4])
    {
        result[0] = ((color >> 16) & 0xff) / 255.0f;
        result[1] = ((color >> 8) & 0xff) / 255.0f;
        result[2] = (color & 0xff) / 255.0f;
        result[3] = ((color >> 24) & 0xff) / 255.0f;
    }
}

cTextureInfo::cTextureInfo(const cRect &Rect, cPoint SurfaceSize)
    : mTop(Rect.top()/(float)SurfaceSize.y)
    , mLeft(Rect.left()/(float)SurfaceSize.x)
    , mBottom((1+Rect.bottom())/(float)SurfaceSize.y)
    , mRight((1+Rect.right())/(float)SurfaceSize.x)
    , mRect(Rect)
{
    mIsWholeSurface=mTop==0.0f&&mLeft==0.0f&&mBottom==1.0f&&mRight==1.0f;
}

cTextureInfo::cTextureInfo(cPoint SurfaceSize)
    : cTextureInfo(cRect({ 0,0 }, SurfaceSize), SurfaceSize)
{
}

cTexture::cTexture(const cTexture &BaseTexture,const cTextureInfo &TextureInfo)
    : mTexture(BaseTexture.mTexture)
    , mShaderResourceView(BaseTexture.mShaderResourceView)
    , mSurface(BaseTexture.mSurface)
    , mSurfaceWidth(BaseTexture.mSurfaceWidth)
    , mSurfaceHeight(BaseTexture.mSurfaceHeight)
    , mTextureInfo(TextureInfo)
{
    if (mTexture)
        mTexture->AddRef();
    if (mShaderResourceView)
        mShaderResourceView->AddRef();
    if (mSurface)
        mSurface->AddRef();
}

cTexture::cTexture(ID3D11Texture2D *BaseTexture,int TextureWidth,int TextureHeight)
    : mTexture(BaseTexture)
    , mTextureInfo({ TextureWidth,TextureHeight })
    , mSurfaceWidth(TextureWidth)
    , mSurfaceHeight(TextureHeight)
{
    ASSERT(mTexture);
    mTexture->AddRef();
    D3V(cDevice::Get()->GetD3DObject()->CreateShaderResourceView(mTexture, nullptr, &mShaderResourceView));
}

tIntrusivePtr<cTexture> cTexture::CreateSubTexture(const cRect &SubRect) const
{
    cTextureInfo SubTextureInfo(SubRect, { mSurfaceWidth, mSurfaceHeight });
    return tIntrusivePtr<cTexture>(new cTexture(*this, SubTextureInfo));
}

tIntrusivePtr<cTexture> cTexture::CreateFlipped(unsigned int FlipFlags) const
{
    cTextureInfo FlippedInfo=mTextureInfo;
    if(FlipFlags&Flip_Horizontal)
        std::swap(FlippedInfo.mLeft, FlippedInfo.mRight);
    if(FlipFlags&Flip_Vertical)
        std::swap(FlippedInfo.mTop, FlippedInfo.mBottom);
    return tIntrusivePtr<cTexture>(new cTexture(*this, FlippedInfo));
}

cTexture::~cTexture()
{
    if (mSurface)
        mSurface->Release();
    if (mShaderResourceView)
        mShaderResourceView->Release();
    if (mTexture)
        mTexture->Release();

    mSurface = nullptr;
    mShaderResourceView = nullptr;
    mTexture = nullptr;
}

void cTexture::InitForRenderTarget(cPoint Size)
{
    mTextureInfo=cTextureInfo(Size);
    mSurfaceWidth=Size.x;
    mSurfaceHeight=Size.y;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = Size.x;
    desc.Height = Size.y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TextureFormat;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    auto device = cDevice::Get()->GetD3DObject();
    D3V(device->CreateTexture2D(&desc, nullptr, &mTexture));
    D3V(device->CreateShaderResourceView(mTexture, nullptr, &mShaderResourceView));
    D3V(device->CreateRenderTargetView(mTexture, nullptr, &mSurface));
}

void cTexture::InitForWritable(cPoint Size)
{
    mTextureInfo=cTextureInfo(Size);
    mSurfaceWidth=Size.x;
    mSurfaceHeight=Size.y;
    mIsWritable = true;
    mWritablePixels.resize(static_cast<size_t>(Size.x) * static_cast<size_t>(Size.y) * 4);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = Size.x;
    desc.Height = Size.y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TextureFormat;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    auto device = cDevice::Get()->GetD3DObject();
    D3V(device->CreateTexture2D(&desc, nullptr, &mTexture));
    D3V(device->CreateShaderResourceView(mTexture, nullptr, &mShaderResourceView));
}

cTexture::cLockInfo cTexture::LockSurface(IsReadOnly isReadOnly)
{
    ASSERT(mIsWritable);
    ASSERT(!mWritablePixels.empty());

    mWritableLockIsReadOnly = isReadOnly == IsReadOnly::yes;

    cLockInfo LockInfo;
    LockInfo.mPitch = mSurfaceWidth * 4;
    LockInfo.mBytes = reinterpret_cast<char *>(mWritablePixels.data());
    return LockInfo;
}

void cTexture::UnlockSurface()
{
    ASSERT(mIsWritable);

    if (!mWritableLockIsReadOnly)
    {
        cDevice::Get()->GetDeviceContext()->UpdateSubresource(
            mTexture,
            0,
            nullptr,
            mWritablePixels.data(),
            mSurfaceWidth * 4,
            0);
    }

    mWritableLockIsReadOnly = false;
}

tIntrusivePtr<cTexture> cTexture::CreateRenderTarget(cPoint Size)
{
    tIntrusivePtr<cTexture> NewTexture(new cTexture(cTextureInfo(Size)));
    NewTexture->InitForRenderTarget(Size);
    return NewTexture;
}

tIntrusivePtr<cTexture> cTexture::CreateWriteable(cPoint Size)
{
    tIntrusivePtr<cTexture> NewTexture(new cTexture(cTextureInfo(Size)));
    NewTexture->InitForWritable(Size);
    return NewTexture;
}

void cTexture::SetAsRenderTarget()
{
    ASSERT(mSurface);
    auto context = theDevice->GetDeviceContext();
    ID3D11ShaderResourceView *nullView = nullptr;
    context->PSSetShaderResources(0, 1, &nullView);
    context->OMSetRenderTargets(1, &mSurface, nullptr);
    Clear(0);
}

void cTexture::Clear(uint32_t Color)
{
    ASSERT(mSurface);
    float color[4];
    ArgbToFloat4(Color, color);
    theDevice->GetDeviceContext()->ClearRenderTargetView(mSurface, color);
}
