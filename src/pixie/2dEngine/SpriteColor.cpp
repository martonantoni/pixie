#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
bool cSpriteColor::CheckPerCornerUsage(unsigned int PositionFlags)
{
// going from PerCornerMode to non-PerCornerMode is tricky, because of setting Alpha / RGB separately

	if(PositionFlags==Entire&&!mPerCornerMode)
		return false;

	if(!mPerCornerMode)
	{
		mCornerColors[0]=mCornerColors[1]=mCornerColors[2]=mCornerColors[3]=mColor;
		mPerCornerMode=true;
	}
	return true;
}

const cColor &cSpriteColor::GetColor_ByCorner(int cornerIndex) const
{
	return mPerCornerMode?mCornerColors[cornerIndex]:*this;
}

#define SPRITE_COLOR_SET_FUNCTION_BODY(ColorFunctionToCall) \
	if(!CheckPerCornerUsage(PositionFlags)) \
	{ \
		ColorFunctionToCall; \
		return; \
	} \
	if(PositionFlags&TopLeft) mCornerColors[CornerPosition::TopLeft].ColorFunctionToCall; \
	if(PositionFlags&TopRight) mCornerColors[CornerPosition::TopRight].ColorFunctionToCall; \
	if(PositionFlags&BottomLeft) mCornerColors[CornerPosition::BottomLeft].ColorFunctionToCall; \
	if(PositionFlags&BottomRight) mCornerColors[CornerPosition::BottomRight].ColorFunctionToCall; 


void cSpriteColor::SetRGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color)
{
	ASSERT(PositionFlags>4); // probably used Corner_ enums instead of flags
	SPRITE_COLOR_SET_FUNCTION_BODY(SetRGBColor(Color));
}

void cSpriteColor::SetARGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color)
{
	ASSERT(PositionFlags>4); // probably used Corner_ enums instead of flags
	SPRITE_COLOR_SET_FUNCTION_BODY(SetARGBColor(Color));
}

void cSpriteColor::SetAlpha_ByPosition(unsigned int PositionFlags,unsigned int Alpha)
{
	ASSERT(PositionFlags>4); // probably used Corner_ enums instead of flags
	SPRITE_COLOR_SET_FUNCTION_BODY(SetAlpha(Alpha));
}

void cSpriteColor::SetAlpha(unsigned int Alpha)
{
	if(mPerCornerMode)
		SetAlpha_ByPosition(Entire,Alpha);
	else
		cColor::SetAlpha(Alpha);
}

const cColor &cSpriteColor::GetColor_ByPosition(unsigned int PositionFlags) const
{
	ASSERT(PositionFlags>4); // probably used Corner_ enums instead of flags
	if(!mPerCornerMode)
		return *this;
	if(PositionFlags&TopLeft) return mCornerColors[CornerPosition::TopLeft];
	if(PositionFlags&TopRight) return mCornerColors[CornerPosition::TopRight];
	if(PositionFlags&BottomLeft) return mCornerColors[CornerPosition::BottomLeft];
	if(PositionFlags&BottomRight) return mCornerColors[CornerPosition::BottomRight];
	return *this;
}

