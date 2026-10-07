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