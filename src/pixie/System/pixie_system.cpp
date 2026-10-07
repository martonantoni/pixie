#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cPixieSystem thePixieSystem;

void cPixieSystem::init()
{
    mMainLoop = std::make_unique<cPixieMainLoop>();
    mMainLoop->init();
}
