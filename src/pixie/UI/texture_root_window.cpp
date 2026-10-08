#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cTextureRootWindow::cTextureRootWindow()
{
    mTexture = cTexture::CreateRenderTarget(1, 1);
    setRenderSurface(mTexture->renderTargetView());
    mRendererId = theRenderers.Register([this]() { render(); });
}

void cTextureRootWindow::PropertiesChanged(unsigned int properties)
{
    cRootWindow::PropertiesChanged(properties);
    if (properties & cPixieObject::Property_Size)
    {
        auto size = GetPlacement().size();
        mTexture->resize(size);
        setRenderSurface(mTexture->renderTargetView());
    }
}