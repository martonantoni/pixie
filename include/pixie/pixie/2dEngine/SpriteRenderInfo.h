#pragma once

#include "pixie/pixie/2dEngine/SpriteColor.h"

struct cSpriteRenderInfo
{
	cRect mRect;
    cSpriteColor::cCornerColors mCornerColors;
	const cTexture *mTexture=nullptr;
	enum eBlendingMode
	{
		Blend_Normal,
		Blend_LikeLight,
		Blend_CopySource,
		Blend_Solid,
		Invalid_Blend_Mode,
	} mBlendingMode=Blend_Normal;
	float mRotation=0; // in degrees
	cSpriteRenderInfo(const cRect &Rect, float Rotation, const cTexture *Texture, eBlendingMode BlendingMode): mRect(Rect), mRotation(Rotation), mTexture(Texture), mBlendingMode(BlendingMode) {}
	cSpriteRenderInfo()=default;
};