#pragma once

class cMultiSprite_Horiz3: public cAutoMultiSpriteBase
{
	enum eSpriteIndex { Sprite_Left,Sprite_Center,Sprite_Right,   NumberOfSprites };
	int mLeftBorderWidth,mRightBorderWidth,mHeight;
	virtual void ArrangeSprites() override;
	virtual const char **GetPositionNames() const override;
	virtual bool FinalizeInit() override;
protected:
	virtual ~cMultiSprite_Horiz3() {}
public:
	cMultiSprite_Horiz3();
};

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSprite_Horiz3, cAutoMultiSpriteBase)