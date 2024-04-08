#pragma once

class cFontManager2
{
	struct cFontData
	{
		std::string mName;
		std::string mAliasOf;
		std::unique_ptr<cFont2> mFont;
		cFontData(const cFontData &)=delete;
		cFontData(cFontData &&)=default;
		cFontData(const std::string &Name): mName(Name) {}
	};
	std::vector<cFontData> mFonts;
	bool InitFont(cFont2 &Font, const cConfig2& Config);
public:
	void Init();
	cFont2 *GetFont(const std::string &Name);
};

extern cFontManager2 theFontManager;