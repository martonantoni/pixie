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

	struct cVariableFontData
	{
		std::filesystem::path mPath;
		std::unordered_map<int, std::weak_ptr<const cFont>> mFonts;
	};
	std::unordered_map<std::string, cVariableFontData> mVariableFonts;

	std::vector<wchar_t> mExtraLetters;
	std::shared_ptr<const cFont> makeFont(const std::string& fileName, int height);
public:
	void Init();
	std::shared_ptr<const cFont> font(const std::string &Name);
	std::shared_ptr<const cFont> createFont(const std::string& name, int size);
};

extern cFontManager theFontManager;