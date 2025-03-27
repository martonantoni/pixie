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
	struct CornerPosition
	{
		static constexpr int TopLeft = 0;
		static constexpr int TopRight = 1;
		static constexpr int BottomLeft = 2;
		static constexpr int BottomRight = 3;
	};
	using cCornerColors = std::array<cColor, 4>;
private:
	cCornerColors mCornerColors;
	bool mPerCornerMode;
	bool CheckPerCornerUsage(unsigned int PositionFlags);
public:
	cSpriteColor(): mPerCornerMode(false) {}
	cSpriteColor(cColor &ColorInfo): cColor(ColorInfo), mPerCornerMode(false) {}
	void SetRGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color);
	void SetARGBColor_ByPosition(unsigned int PositionFlags,unsigned int Color);
	void SetAlpha_ByPosition(unsigned int PositionFlags,unsigned int Alpha);
	cCornerColors cornerColors() const;
	const cColor &GetColor_ByPosition(unsigned int PositionFlag) const;
	const cColor &GetColor_ByCorner(int cornerIndex) const;
	const cSpriteColor &operator=(const cColor &ColorInfo) { mColor=GetARGBColor(); mPerCornerMode=false; return *this; }
	void SetAlpha(unsigned int Alpha); // 0 - 255, 0: solid, 255: transparent
};
