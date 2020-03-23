#pragma once

struct cTextureFile;
class cTextureManager;
class cSpriteRenderer;
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
	friend cSpriteRenderer;
	IDirect3DTexture9 *mTexture=nullptr;
	int mSurfaceWidth,mSurfaceHeight;
	cTextureInfo mTextureInfo;
protected:
	bool mNeedUpdateBeforeUse=false;
	IDirect3DSurface9 *mSurface=nullptr; // level 0 of the mTexture
	cTexture(const cTextureInfo &TextureInfo): mTextureInfo(TextureInfo) {}
	~cTexture();
	void InitForRenderTarget(cPoint Size);
	void InitForWritable(cPoint Size);
public:
	cTexture(const cTexture &BaseTexture,const cTextureInfo &TextureInfo);
	cTexture(IDirect3DTexture9 *BaseTexture,int TextureWidth,int TextureHeight);
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
	int GetSurfaceWidth() const { return mSurfaceWidth; }   // the entire surface, might be bigger than the area the texture occupies
	int GetSurfaceHeight() const { return mSurfaceHeight; }
	int GetTextureWidth() const { return  mTextureInfo.mRect.GetSize().x; }
	int GetTextureHeight() const { return mTextureInfo.mRect.GetSize().y; }
	cRect GetTextureRect() const { return mTextureInfo.mRect; }
	cPoint GetSize() const { return { GetTextureWidth(),GetTextureHeight() }; }

// functions reserved for Textures created with CreateRenderTarget:
	void SetAsRenderTarget(); 
	bool DoesNeedUpdateBeforeUse() const { return mNeedUpdateBeforeUse; }
	virtual void Update() {}
	void Clear(D3DCOLOR Color);
// functions reserved for Textures created with CreateWritable:
	struct cLockInfo
	{
		int mPitch;
		char *mBytes=nullptr;
	};
	cLockInfo LockSurface(); // works only if texture was created with CreateWritable
	void UnlockSurface(); 
};

