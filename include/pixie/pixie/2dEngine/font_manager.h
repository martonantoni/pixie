#pragma once

class cFontManager
{
	struct cFontData
	{
		std::string mName;
		std::string mAliasOf;
		std::shared_ptr<const cFont> mFont;
		cFontData(const cFontData &)=delete;
		cFontData(cFontData &&)=default;
		cFontData(const std::string &Name): mName(Name) {}
	};
	std::vector<cFontData> mFonts;
	std::vector<wchar_t> mExtraLetters;
	std::shared_ptr<const cFont> makeFont(const std::string& fileName, int height);
public:
	void Init();
	std::shared_ptr<const cFont> GetFont(const std::string &Name);
	std::shared_ptr<const cFont> createFont(const std::string& name, int size);
};

extern cFontManager theFontManager;