#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cVisualizerBase::RedrawNeeded()
{
	if(!mRedrawID.IsValid())
	{
		mRedrawID=theRenderers.Register([this]() { mRedrawID.Unregister(); Redraw(); });
	}
}
