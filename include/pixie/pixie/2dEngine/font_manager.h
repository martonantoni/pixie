#pragma once

class cFontManager
{
	struct cFontData
	{
		std::string mName;
		std::string mAliasOf;
		std::shared_ptr<cFont> mFont;
		cFontData(const cFontData &)=delete;
		cFontData(cFontData &&)=default;
		cFontData(const std::string &Name): mName(Name) {}
	};
	std::vector<cFontData> mFonts;
	bool InitFont(cFont &Font, const cConfig& Config);
public:
	void Init();
	std::shared_ptr<const cFont> GetFont(const std::string &Name);
};

extern cFontManager theFontManager;