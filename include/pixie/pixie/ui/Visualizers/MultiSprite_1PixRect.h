#pragma once

class cMultiSprite_1PixRect: public cMultiSpriteBase
{
public:
	enum eSpriteIndex { 
		Sprite_Top, Sprite_Bottom, Sprite_Left, Sprite_Right, Sprite_Center,
		
		NumberOfSprites };
	enum class eOnlyBorder { Yes, No };
private:
	eOnlyBorder mOnlyBorders;
	virtual void ArrangeSprites() override;
protected:
	virtual ~cMultiSprite_1PixRect()=default;
public:
	cMultiSprite_1PixRect(const std::string &ColorNameBase, eOnlyBorder OnlyBorder=eOnlyBorder::No);
};

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSprite_1PixRect, cMultiSpriteBase)
