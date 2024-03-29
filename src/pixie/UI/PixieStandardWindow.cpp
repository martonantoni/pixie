#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cStandardWindow);

void cStandardWindow::Init(const cInitData &InitData)
{
	cPixieWindow::Init(InitData);
	InitVisualizer(InitData, InitData.mVisualizer);
}
