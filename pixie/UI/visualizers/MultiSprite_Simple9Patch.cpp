#include "StdAfx.h"

cMultiSprite_Simple9Patch::cMultiSprite_Simple9Patch()
{
}

void cMultiSprite_Simple9Patch::ArrangeSprites()
{
	cPoint Position(GetPosition()),Size(GetSize());
	mSprites[Sprite_TopLeft]->SetPosition(Position);
	mSprites[Sprite_BottomLeft]->SetPosition(Position+cPoint(0,Size.y-mSprites[Sprite_BottomLeft]->GetHeight()));
	mSprites[Sprite_TopRight]->SetPosition(Position+cPoint(Size.x-mSprites[Sprite_TopRight]->GetWidth(),0));
	mSprites[Sprite_BottomRight]->SetPosition(Position+cPoint(Size.x-mSprites[Sprite_BottomRight]->GetWidth(),Size.y-mSprites[Sprite_BottomRight]->GetHeight()));

	mSprites[Sprite_Top]->SetPosition(Position+cPoint(mSprites[Sprite_TopLeft]->GetWidth(),0));
	mSprites[Sprite_Top]->SetSize(Size.x-mSprites[Sprite_TopLeft]->GetWidth()-mSprites[Sprite_TopRight]->GetWidth(),mSprites[Sprite_Top]->GetHeight());

	mSprites[Sprite_Bottom]->SetPosition(Position+cPoint(mSprites[Sprite_BottomLeft]->GetWidth(),Size.y-mSprites[Sprite_Bottom]->GetHeight()));
	mSprites[Sprite_Bottom]->SetSize(Size.x-mSprites[Sprite_BottomLeft]->GetWidth()-mSprites[Sprite_BottomRight]->GetWidth(),mSprites[Sprite_Bottom]->GetHeight());

	mSprites[Sprite_Left]->SetPosition(Position+cPoint(0,mSprites[Sprite_TopLeft]->GetHeight()));
	mSprites[Sprite_Left]->SetSize(mSprites[Sprite_Left]->GetWidth(),Size.y-mSprites[Sprite_TopLeft]->GetHeight()-mSprites[Sprite_BottomLeft]->GetHeight());

	mSprites[Sprite_Right]->SetPosition(Position+cPoint(Size.x-mSprites[Sprite_Right]->GetWidth(),mSprites[Sprite_TopRight]->GetHeight()));
	mSprites[Sprite_Right]->SetSize(mSprites[Sprite_Right]->GetWidth(),Size.y-mSprites[Sprite_TopRight]->GetHeight()-mSprites[Sprite_BottomRight]->GetHeight());

 	if(mSprites[Sprite_Middle])
 	{
		switch(mCenterSpriteMode)
		{
		case eCenterSpriteMode::BetweenSides:
			{
				cPoint MiddleSize(Size.x-mSprites[Sprite_Left]->GetSize().x-mSprites[Sprite_Right]->GetSize().x,
					Size.y-mSprites[Sprite_Top]->GetSize().y-mSprites[Sprite_Bottom]->GetSize().y);
				cRect MiddleRect(GetPosition(), MiddleSize);
				MiddleRect.Move({ mSprites[Sprite_Left]->GetSize().x, mSprites[Sprite_Top]->GetSize().y });
				mSprites[Sprite_Middle]->SetRect(MiddleRect);
				break;
			}
		case eCenterSpriteMode::Entire:
			{
				mSprites[Sprite_Middle]->SetRect(GetRect());
				break;
			}
		case eCenterSpriteMode::None:
			{
				mSprites[Sprite_Middle].reset();
				break;
			}
		}
 	}
}

const char **cMultiSprite_Simple9Patch::GetPositionNames() const
{
	static const char *PositionNames[]={
		"top_left", "top", "top_right",
		"left", "*middle", "right",    // middle one is optional
		"bottom_left", "bottom", "bottom_right",
		nullptr };
	return mPositionNamesOverrides?mPositionNamesOverrides:PositionNames;
}

LuaObject cMultiSprite_Simple9Patch::Lua_Get9PatchNames()
{
	LuaObject Table=theLuaState->CreateTable();
	static const char *PositionNames[]= {
		"top_left", "top", "top_right",
		"left", "middle", "right",   
		"bottom_left", "bottom", "bottom_right",
		nullptr };
	for(int i=0; PositionNames[i]; ++i)
	{
		Table.Set(i+1, PositionNames[i]);
	}
	return Table;
}
REGISTER_LUA_FUNCTION(cMultiSprite_Simple9Patch::Lua_Get9PatchNames, Get9PatchNames);

void cMultiSprite_Simple9Patch::SetPositionNameOverrides(const char **PositionNameOverrides)
{
	mPositionNamesOverrides=PositionNameOverrides;
}

void cMultiSprite_Simple9Patch::SetCenterSpriteMode(eCenterSpriteMode CenterSpriteMode)
{
	mCenterSpriteMode=CenterSpriteMode;
}


bool cMultiSprite_Simple9Patch::FinalizeInit()
{
	if(mSprites[Sprite_Middle])
	{
		mSprites[Sprite_Middle]->SetZOrder(mSprites[Sprite_Middle]->GetZOrder()-1);
	}
	return true;
}


