#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cRootWindow::cRootWindow()
{
    mRenderer = std::make_unique<cSpriteRenderer>(*this);
}

void cRootWindow::render()
{
    if (mRenderer)
        mRenderer->Render();
}

void cRootWindow::setRenderSurface(ID3D11RenderTargetView* renderSurface)
{
    if (mRenderer)
        mRenderer->SetRenderSurface(renderSurface);
}

void cRootWindow::renderSurfaceSizeChanged(cPoint newSize)
{
    if (mRenderer)
        mRenderer->renderSurfaceSizeChanged(newSize);
}
