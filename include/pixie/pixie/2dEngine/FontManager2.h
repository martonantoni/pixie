#pragma once

class cFontManager
{
	struct cFontData
	{
		std::string mName;
		std::string mAliasOf;
		std::unique_ptr<cFont> mFont;
		cFontData(const cFontData &)=delete;
		cFontData(cFontData &&)=default;
		cFontData(const std::string &Name): mName(Name) {}
	};
	std::vector<cFontData> mFonts;
	bool InitFont(cFont &Font, const cConfig& Config);
public:
	void Init();
	cFont *GetFont(const std::string &Name);
};

extern cFontManager theFontManager;