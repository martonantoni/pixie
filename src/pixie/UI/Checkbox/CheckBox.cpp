#include "StdAfx.h"

INIT_VISUALIZER_IMP(cCheckBox);

cCheckBox::cCheckBox()
: mIsChecked(false)
{
}

void cCheckBox::Init(const cInitData &InitData)
{
	cDialogItem::Init(InitData);
	InitVisualizer(InitData, InitData.mVisualizer);
}

void cCheckBox::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	StartMouseTracking();
}

void cCheckBox::OnLeftButtonUp(cPoint ScreenCoords, bool IsInside)
{
	if(IsMouseTrackingActive())
	{
		StopMouseTracking();
		if(IsInside)
		{
			//		mEventDispatchers.PostEvent(Event_Activated);
			Selected();
			mVisualizer->StateChanged();
		}
	}
}

void cCheckBox::Selected()
{
	mIsChecked=!mIsChecked;
}

void cCheckBox::SetCheckState(bool IsChecked)
{
	mIsChecked=IsChecked;
	mVisualizer->StateChanged();
}

void cCheckBox::DialogItemStateChanged()
{
	mVisualizer->StateChanged();
}
