#pragma once

class cFontManager: public tSingleton<cFontManager>
{
	typedef std::map<std::string,cFont *> cFontMap;
	cFontMap mFontMap;
public:
	~cFontManager();
	void RegisterFonts(cConfig &FontsConfig);
	void RegisterFont(cFont *Font, const std::string &FontName);
	cFont *GetFont(const std::string &FontName);
};