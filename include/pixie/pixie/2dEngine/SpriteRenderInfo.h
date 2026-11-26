#pragma once


struct cSpriteRenderInfo
{
	cRect mRect;
	cColor mColor;
    const cTextureBindings* mTextureBindings = nullptr;
	cPixelShader* mShader = nullptr;
    float mShaderParameters[4] = { 0, 0, 0, 0 };
	enum eBlendingMode
	{
		Blend_Normal,
		Blend_CopySource,
		Invalid_Blend_Mode,
	} mBlendingMode=Blend_Normal;
	float mRotation=0; // in degrees
    bool mEnableRender = false;
	cSpriteRenderInfo()=default;
};