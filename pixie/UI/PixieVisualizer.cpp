#include "StdAfx.h"

void cVisualizerBase::RedrawNeeded()
{
	if(!mRedrawID.IsValid())
	{
		mRedrawID=theRenderers.Register([this]() { mRedrawID.Unregister(); Redraw(); });
	}
}
