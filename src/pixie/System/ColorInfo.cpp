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

cColor cColor::toGrayscale() const
{
    int gray = (0.2126f * GetRed() + 0.7152f * GetGreen() + 0.0722f * GetBlue());
    return cColor(gray, gray, gray);
}
