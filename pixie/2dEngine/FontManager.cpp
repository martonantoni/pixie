#include "StdAfx.h"

cFontManager::~cFontManager()
{
}

void cFontManager::RegisterFonts(cConfig &FontsConfig)
{
	auto FontConfigNames=FontsConfig.GetSubConfigNames();
	for(auto &ConfigName: FontConfigNames)
	{
		RegisterFont(new cFont(*FontsConfig.GetSubConfig(ConfigName)), ConfigName);
	}
}

cFont *cFontManager::GetFont(const std::string &FontName)
{
	cFontMap::iterator i=mFontMap.find(FontName);
	ASSERT(i!=mFontMap.end());
	return i->second;
}

void cFontManager::RegisterFont(cFont *Font,const std::string &FontName)
{
	mFontMap[FontName]=Font;
}
