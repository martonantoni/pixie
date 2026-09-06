#pragma once

class cColor
{
protected:
	unsigned int mColor;
    cColor adjustedRGB(double amount) const;
    cColor adjustedHSV(double amount) const;
    cColor adjustedHSL(double amount) const;
public:
	cColor(unsigned int Color=0xff000000): mColor(Color) {}
	cColor(int Red, int Green, int Blue): mColor(0xff000000|((Red&0xff)<<16)|((Green&0xff)<<8)|(Blue&0xff)) {}
	cColor(auto&& colorName) requires std::totally_ordered_with<decltype(colorName), std::string>;
	cColor(const cColor &ColorInfo)=default;
	cColor(cColor &&)=default;
	cColor &operator=(const cColor &)=default;
	cColor &operator=(cColor &&)=default;

	struct RGB {};
	struct HSV {};
	struct HSL {};

	template<class MODE = RGB> [[nodiscard]] cColor adjusted(double amount) const;

	unsigned int GetAlpha() const { return 255-(mColor>>24); } // 0 - 255, 0: solid, 255: transparent
	unsigned int GetRGBColor() const { return mColor&0xffffff; }
	unsigned int GetARGBColor() const { return mColor; }
	void SetAlpha(unsigned int Alpha) { ASSERT(Alpha<=255); mColor=(mColor&0xffffff)|((255-Alpha)<<24); } // 0 - 255, 0: solid, 255: transparent
	void SetRGBColor(unsigned int Color) { mColor=(mColor&0xff000000)|(Color&0xffffff); }
	void SetARGBColor(unsigned int Color) { mColor=Color; }

	int GetRed() const { return (mColor>>16)&0xff; }
	int GetGreen() const { return (mColor>>8)&0xff; }
	int GetBlue() const { return mColor&0xff; }
	void SetRed(int Red) { ASSERT(Red>=0 && Red<=255); mColor=(mColor&0xff00ffff)|((Red&0xff)<<16); }
	void SetGreen(int Green) { ASSERT(Green>=0 && Green<=255); mColor=(mColor&0xffff00ff)|((Green&0xff)<<8); }
	void SetBlue(int Blue) { ASSERT(Blue>=0 && Blue<=255); mColor=(mColor&0xffffff00)|(Blue&0xff); }

	auto red()
	{
		return tProxy<cColor, int,
			[](auto& color) { return color.GetRed(); },
			[](auto& color, auto value) { color.SetRed(value); } > (*this);

	}
    int red() const { return GetRed(); }
    auto green()
    {
        return tProxy<cColor, int,
            [](auto& color) { return color.GetGreen(); },
            [](auto& color, auto value) { color.SetGreen(value); } > (*this);
    }
    int green() const { return GetGreen(); }
    auto blue()
    {
        return tProxy<cColor, int,
            [](auto& color) { return color.GetBlue(); },
            [](auto& color, auto value) { color.SetBlue(value); } > (*this);
    }
    int blue() const { return GetBlue(); }
    auto alpha()
    {
        return tProxy<cColor, int,
            [](auto& color) { return color.GetAlpha(); },
            [](auto& color, auto value) { color.SetAlpha(value); } > (*this);
    }
    int alpha() const { return GetAlpha(); }

	cColor toGrayscale() const;

	static cColor FromRGBColor(unsigned int Color) { return cColor(Color|0xff000000); }
    bool operator==(const cColor& other) const { return mColor == other.mColor; }
};

template<class MODE> cColor cColor::adjusted(double amount) const
{
    if constexpr (std::same_as<MODE, RGB>)
        return adjustedRGB(amount);
    else if constexpr (std::same_as<MODE, HSV>)
        return adjustedHSV(amount);
    else if constexpr (std::same_as<MODE, HSL>)
        return adjustedHSL(amount);
}


class cColorServer
{
	std::map<std::string, cColor, std::less<>> mColorMap;
public:
	void Init();
	cColor GetColor(auto&& colorName) const requires std::totally_ordered_with<decltype(colorName), std::string>
	{
        auto i = mColorMap.find(std::forward<decltype(colorName)>(colorName));
		if (ASSERTTRUE(i != mColorMap.end()))
            return i->second;
        MainLog->Log("Unknown color name: {}", std::forward<decltype(colorName)>(colorName));
        return cColor(0xffff00ff); // magenta
    }
};

extern cColorServer theColorServer;

cColor::cColor(auto&& colorName) requires std::totally_ordered_with<decltype(colorName), std::string>
    : mColor(theColorServer.GetColor(std::forward<decltype(colorName)>(colorName)).GetARGBColor())
{
}
