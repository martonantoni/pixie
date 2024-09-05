#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cMultiSprite_1PixRect::cMultiSprite_1PixRect(const std::string &ColorNameBase, eOnlyBorder OnlyBorder)
	: mOnlyBorders(OnlyBorder)
{
	auto Texture=theTextureManager.GetTexture("1pix");
	mSprites.reserve(NumberOfSprites);
	cColor BorderColor(ColorNameBase+"_border");
	for(int i=0; i<(OnlyBorder==eOnlyBorder::No?NumberOfSprites:NumberOfSprites-1); ++i)
	{
		auto Sprite=std::make_unique<cSprite>();
		Sprite->SetTexture(Texture);
		mSprites.emplace_back(std::move(Sprite));
	}
	mSprites[Sprite_Top]->SetRGBColor(BorderColor);
	mSprites[Sprite_Left]->SetRGBColor(BorderColor);
	mSprites[Sprite_Bottom]->SetRGBColor(BorderColor);
	mSprites[Sprite_Right]->SetRGBColor(BorderColor);
	if(mOnlyBorders!=eOnlyBorder::Yes)
	{
		cColor InsideColor(ColorNameBase+"_inside");
		mSprites[Sprite_Center]->SetRGBColor(InsideColor);
	}
}

void cMultiSprite_1PixRect::ArrangeSprites()
{
	cRect Rect=GetRect();
	mSprites[Sprite_Top]->SetRect({ Rect.topLeft(), {Rect.width(), 1} });
	mSprites[Sprite_Bottom]->SetRect({ Rect.bottomLeft(), { Rect.width(), 1 } });
	mSprites[Sprite_Left]->SetRect({ Rect.topLeft() + cPoint{0,1}, { 1, Rect.height() - 2} });
	mSprites[Sprite_Right]->SetRect({ Rect.topRight() + cPoint { 0,1 }, { 1, Rect.height() - 2 } });
	if (mOnlyBorders != eOnlyBorder::Yes)
		mSprites[Sprite_Center]->SetRect({ Rect.topLeft() + cPoint{1,1},Rect.size() - cPoint{2,2} });
}
