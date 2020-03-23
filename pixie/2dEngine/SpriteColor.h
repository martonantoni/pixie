#pragma once

class cSpriteColor: public cColor
{
public:
	enum // color position flags:
	{ 
		TopLeft           = 0x100,
		BottomLeft        = 0x200, 
		TopRight          = 0x400, 
		BottomRight       = 0x800, 
		Top               = TopLeft|TopRight,
		Bottom            = BottomLeft|BottomRight,
		Left              = TopLeft|BottomLeft,
		Right             = TopRight|BottomRight,
		Entire            = Top|Bottom|Left|Right,
	};
	enum eCornerPosition
	{
		Corner_TopLeft, Corner_TopRight, Corner_BottomLeft, Corner_BottomRight
	};
private:
	cColor mCornerColors[4];
	bool mPerCornerMode;
	bool CheckPerCornerUsage(unsigned int PositionFlags);
public:
	cSpriteColor(): mPerCornerMode(false) {}
	cSpriteColor(cColor &ColorInfo): cColor(ColorInfo), mPerCornerMode(false) {}
	void SetRGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color);
	void SetARGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color);
	void SetAlpha_ByPosition(unsigned int PositionFlags,unsigned int Alpha);
	const cColor &GetColor_ByPosition(unsigned int PositionFlag) const;
	const cColor &GetColor_ByCorner(eCornerPosition Corner) const;
	const cSpriteColor &operator=(const cColor &ColorInfo) { mColor=GetARGBColor(); mPerCornerMode=false; return *this; }
	void SetAlpha(unsigned int Alpha); // 0 - 255, 0: solid, 255: transparent
};
