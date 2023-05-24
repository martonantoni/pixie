#include "StdAfx.h"

cMultiSprite_Horiz3::cMultiSprite_Horiz3()
{
}

const char **cMultiSprite_Horiz3::GetPositionNames() const
{
	static const char *PositionNames[]={ "left", "center", "right", NULL };
	return PositionNames;
}


bool cMultiSprite_Horiz3::FinalizeInit()
{
	mLeftBorderWidth=mSprites[Sprite_Left]->GetPrefferedSize().x;
	mHeight=mSprites[Sprite_Left]->GetPrefferedSize().y;
	mRightBorderWidth=mSprites[Sprite_Right]->GetPrefferedSize().x;

	return true;
}

void cMultiSprite_Horiz3::ArrangeSprites()
{	
	cPoint Position(GetPosition()),Size(GetSize());
	mSprites[Sprite_Left]->SetPosition(Position);
	mSprites[Sprite_Left]->SetSize(mLeftBorderWidth,mHeight);
	mSprites[Sprite_Center]->SetPosition(Position+cPoint(mLeftBorderWidth,0));
	mSprites[Sprite_Center]->SetSize(Size.x-mLeftBorderWidth-mRightBorderWidth,mHeight);
	mSprites[Sprite_Right]->SetPosition(Position+cPoint(Size.x-mRightBorderWidth,0));
	mSprites[Sprite_Right]->SetSize(mRightBorderWidth,mHeight);
}

