#pragma once

class cSpriteRenderer: public cRenderer
{
	struct cSpriteVertexData
	{
		float x;
		float y;
		float z;
		float rhw;
		D3DCOLOR color;
		float u;
		float v;
		static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	};
	struct cRenderState
	{
		int SpriteCount = 0, StateChangeCount = 0, TextureChangeCount = 0;
		IDirect3DTexture9* Texture = nullptr;
		cSpriteRenderInfo::eBlendingMode LastBlendingMode = cSpriteRenderInfo::Invalid_Blend_Mode;
		int NumberOfBatchedVertices = 0;
		cSpriteVertexData* batchVertices;
	};
	bool mIsUnderDestruction=false;
	bool mIsInitDone=false;
	cPixieWindow &mBaseWindow;
	// Direct3D stuff:
	IDirect3DDevice9 *mDevice=nullptr;
	IDirect3DVertexBuffer9 *mVertexBuffer=nullptr;
	IDirect3DIndexBuffer9 *mIndexBuffer=nullptr;
	int mMaxSpritesPerFlush=0;
	bool mClearBeforeRender=false;
	void UpdateBlending(cSpriteRenderInfo::eBlendingMode BlendingMode);
	void FlushBuffer(cSpriteVertexData* batchVertices, int &NumberOfBatchedVertices, bool RelockBuffer);
	void Init();
	void RenderSprites();
	void UpdateUsedTextures();
	static void Rotate(cFloatPoint &Point, cFloatPoint Center, float s, float c);
protected:
	cDevice *mPixieDevice=nullptr;
public:
	cSpriteRenderer(cPixieWindow &BaseWindow): mBaseWindow(BaseWindow) {}
	~cSpriteRenderer();
	void AddSprite(cSprite *Sprite);
	void RemoveSprite(cSprite *Sprite);
	void SetClearBeforeRender(bool ClearBeforeRender) { mClearBeforeRender=ClearBeforeRender; }
	virtual void Render() override;
};
