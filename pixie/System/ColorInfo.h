#pragma once

class cColor
{
protected:
	unsigned int mColor;
public:
	cColor(unsigned int Color=0xff000000): mColor(Color) {}
	cColor(const std::string &ColorName);
	cColor(const char *ColorName): cColor(std::string(ColorName)) {}
	cColor(const cColor &ColorInfo)=default;
	cColor(cColor &&)=default;
	cColor &operator=(const cColor &)=default;
	cColor &operator=(cColor &&)=default;

	unsigned int GetAlpha() const { return 255-(mColor>>24); } // 0 - 255, 0: solid, 255: transparent
	unsigned int GetRGBColor() const { return mColor&0xffffff; }
	unsigned int GetARGBColor() const { return mColor; }
	void SetAlpha(unsigned int Alpha) { ASSERT(Alpha<=255); mColor=(mColor&0xffffff)|((255-Alpha)<<24); } // 0 - 255, 0: solid, 255: transparent
	void SetRGBColor(unsigned int Color) { mColor=(mColor&0xff000000)|(Color&0xffffff); }
	void SetARGBColor(unsigned int Color) { mColor=Color; }

	int GetRed() const { return (mColor>>16)&0xff; }
	int GetGreen() const { return (mColor>>8)&0xff; }
	int GetBlue() const { return mColor&0xff; }

	static cColor FromRGBColor(unsigned int Color) { cColor(Color|0xff000000); }
};

class cColorServer
{
	struct cNamedColor
	{
		std::string mName;
		cColor mColor;
		cNamedColor(const std::string &Name, cColor Color): mName(Name), mColor(Color) {}
		cNamedColor(cNamedColor &&)=default;
	};
	std::vector<cNamedColor> mColors;
public:
	void Init();
	cColor GetColor(const std::string &Name) const;
};

extern cColorServer theColorServer;