#pragma once

class cMultiSprite_Simple9Patch: public cAutoMultiSpriteBase
{
public:
	enum class eCenterSpriteMode { Entire, BetweenSides, None };
	enum eSpriteIndex
	{
		Sprite_TopLeft, Sprite_Top, Sprite_TopRight,
		Sprite_Left, Sprite_Middle, Sprite_Right,
		Sprite_BottomLeft, Sprite_Bottom, Sprite_BottomRight,

		NumberOfSprites
	}; 
private:
	const char **mPositionNamesOverrides=nullptr;
	eCenterSpriteMode mCenterSpriteMode=eCenterSpriteMode::BetweenSides;
	virtual void ArrangeSprites() override;
	virtual const char **GetPositionNames() const override;
	virtual bool FinalizeInit() override;
protected:
	virtual ~cMultiSprite_Simple9Patch() {}
public:
	cMultiSprite_Simple9Patch();
	void SetCenterSpriteMode(eCenterSpriteMode CenterSpriteMode); // must be called before Init
	void SetPositionNameOverrides(const char **PositionNameOverrides); // must be called before Init

//	static LuaObject Lua_Get9PatchNames();
};

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSprite_Simple9Patch, cAutoMultiSpriteBase)
