#pragma once


struct cSpriteRenderInfo
{
	cRect mRect;
	cColor mColor;
	const cTexture *mTextures[4] = { nullptr, nullptr, nullptr, nullptr };
    int mNumberOfTextures = 1;
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
		mTextures{Texture, nullptr, nullptr, nullptr}, 
		mNumberOfTextures(1), 
		mBlendingMode(BlendingMode) {}
	cSpriteRenderInfo()=default;
    decltype(mTextures[0]->mShaderResourceView) shaderResourceView(int index) const
    {
        if (index < 0 || index >= mNumberOfTextures)
            return nullptr;
        return mTextures[index] ? mTextures[index]->mShaderResourceView : nullptr;
    }
};