#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cColorServer theColorServer;

void cColorServer::Init()
{
	auto colorsConfig = theGlobalConfig->GetSubConfig("colors");
	for(auto &name: colorsConfig->GetKeys())
	{
		mColorMap[name] = cColor(colorsConfig->get<int>(name));
	}
}

cColor cColorServer::GetColor(const std::string& name) const
{
	auto i = mColorMap.find(name);
	ASSERT(i != mColorMap.end());
	return i->second;
}

cColor::cColor(const std::string &ColorName)
	: cColor(theColorServer.GetColor(ColorName))
{
}
