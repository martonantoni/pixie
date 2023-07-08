#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cColorServer theColorServer;

void cColorServer::Init()
{
	auto FontsConfig= theGlobalConfig->GetSubConfig("colors");
	auto Names=FontsConfig->GetKeys();
	for(auto &Name: Names)
	{
		mColors.emplace_back(Name, FontsConfig->GetInt(Name));
	}
}

cColor cColorServer::GetColor(const std::string &Name) const
{
 	auto i=std::find_if(mColors, [&Name](auto &ColorInfo) {return ColorInfo.mName==Name; });
 	return ASSERTFALSE(i==mColors.end())?cColor():i->mColor;
}

cColor::cColor(const std::string &ColorName)
	: cColor(theColorServer.GetColor(ColorName))
{
}
