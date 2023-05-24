#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cTextureInfo::cTextureInfo(const cRect &Rect, cPoint SurfaceSize)
	: mTop(Rect.Top()/(float)SurfaceSize.y)
	, mLeft(Rect.Left()/(float)SurfaceSize.x)
	, mBottom((1+Rect.Bottom()/*+0.5*/)/(float)SurfaceSize.y)
	, mRight((1+Rect.Right()/*+0.5*/)/(float)SurfaceSize.x)
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
, mTextureInfo(TextureInfo)
, mSurfaceWidth(BaseTexture.mSurfaceWidth)
, mSurfaceHeight(BaseTexture.mSurfaceHeight)
{
	mTexture->AddRef();
}

cTexture::cTexture(IDirect3DTexture9 *BaseTexture,int TextureWidth,int TextureHeight)
: mTexture(BaseTexture)
, mTextureInfo({ TextureWidth,TextureHeight })
, mSurfaceWidth(TextureWidth)
, mSurfaceHeight(TextureHeight)
{
	mTexture->AddRef();
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
	if(mSurface)
	{
		mSurface->Release();
		mSurface=nullptr;
	}
	mTexture->Release();
	mTexture=nullptr;
}

void cTexture::InitForRenderTarget(cPoint Size)
{
	mTextureInfo=cTextureInfo(Size);
	mSurfaceWidth=Size.x;
	mSurfaceHeight=Size.y;

	D3V(cDevice::Get()->GetD3DObject()->CreateTexture(Size.x, Size.y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8
		/*D3DFMT_R5G6B5*/
		, D3DPOOL_DEFAULT, &mTexture, nullptr));
	mTexture->GetSurfaceLevel(0, &mSurface);
}

void cTexture::InitForWritable(cPoint Size)
{
	mTextureInfo=cTextureInfo(Size);
	mSurfaceWidth=Size.x;
	mSurfaceHeight=Size.y;

	D3V(cDevice::Get()->GetD3DObject()->CreateTexture(Size.x, Size.y, 1, 0 /*usage*/, D3DFMT_A8R8G8B8
		/*D3DFMT_R5G6B5*/
		, D3DPOOL_MANAGED, &mTexture, nullptr));
	mTexture->GetSurfaceLevel(0, &mSurface);
}

cTexture::cLockInfo cTexture::LockSurface(IsReadOnly isReadOnly)
{
    if (!mSurface)
    {
        mTexture->GetSurfaceLevel(0, &mSurface);
    }
	D3DLOCKED_RECT LockedRect;
	auto result=mSurface->LockRect(&LockedRect, NULL, isReadOnly == IsReadOnly::yes ? D3DLOCK_READONLY : 0);
	if(result!=S_OK)
	{
		return cLockInfo();
	}
	cLockInfo LockInfo;
	LockInfo.mPitch=LockedRect.Pitch;
	LockInfo.mBytes=(char *)LockedRect.pBits;
	return LockInfo;
}

void cTexture::UnlockSurface()
{
	mSurface->UnlockRect();
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
	D3V(theDevice->GetD3DObject()->SetRenderTarget(0, mSurface));
	Clear(D3DCOLOR_ARGB(0,0,0,0));
}

void cTexture::Clear(D3DCOLOR Color)
{
	StopOnError(theDevice->GetD3DObject()->Clear(0,  //Number of rectangles to clear, we're clearing everything so set it to 0
		nullptr, //Pointer to the rectangles to clear, NULL to clear whole display
		D3DCLEAR_TARGET,   //What to clear.  We don't have a Z Buffer or Stencil Buffer
		Color, //Colour to clear to (AARRGGBB)
		1.0f,  //Value to clear ZBuffer to, doesn't matter since we don't have one
		0 ));   //Stencil clear value, again, we don't have one, this value doesn't matter
}