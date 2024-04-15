#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cColorServer theColorServer;

void cColorServer::Init()
{
	theGlobalConfig->createSubConfig("colors")->forEachInt([&](const std::string& name, int color)
    {
        mColorMap[name] = cColor(color);
    });
}

cColor cColorServer::GetColor(const std::string& name) const
{
	auto i = mColorMap.find(name);
	ASSERT(i != mColorMap.end());
	if(i == mColorMap.end())
    {
        return cColor(0u);
    }
	return i->second;
}

cColor::cColor(const std::string &ColorName)
	: cColor(theColorServer.GetColor(ColorName))
{
}
