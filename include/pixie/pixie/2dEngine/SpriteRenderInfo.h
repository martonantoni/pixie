#pragma once


struct cSpriteRenderInfo
{
	cRect mRect;
	cColor mColor;
    cTextureResourceHandle mTextures[4] = { nullptr, nullptr, nullptr, nullptr };
	cTextureRect mTextureRects[4];
	cPixelShader* mShader = nullptr;
    float mShaderParameters[4] = { 0, 0, 0, 0 };
	enum eBlendingMode
	{
		Blend_Normal,
		Blend_CopySource,
		Invalid_Blend_Mode,
	} mBlendingMode=Blend_Normal;
	float mRotation=0; // in degrees
	cSpriteRenderInfo(const cRect &Rect, float Rotation, const cTexture *Texture, eBlendingMode BlendingMode): 
		mRect(Rect), 
		mRotation(Rotation), 
		mTextures{Texture ? Texture->shaderResourceView() : nullptr, nullptr, nullptr, nullptr}, 
        mTextureRects{ Texture ? Texture->GetTextureInfo() : cTextureRect(), cTextureRect(), cTextureRect(), cTextureRect() },
		mBlendingMode(BlendingMode) {}
	cSpriteRenderInfo()=default;
	cTextureResourceHandle shaderResourceView(int index) const
    {
        if (index < 0 || index >= 4)
			return nullptr;
        return mTextures[index];
    }
};