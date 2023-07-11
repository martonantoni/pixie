#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
INIT_VISUALIZER_IMP(cPushButton);

const cEventDispatchers::cDispatcherRangeInfo cPushButton::mDispatcherRangeInfo=
{
	PixieEvents::PushButton_First, { "activated" }
};

cPushButton::cInitData::cInitData(const cConfig &config): tPixieSimpleInitData<cMouseTarget>(config)
{
    mTextureBaseName = config.GetString("texture", std::string());
    mTitle = config.GetString("title", std::string());
    mTextStyle = config.GetString("text_style", mTextStyle);
    auto ActivationKeyString = config.GetString("key", std::string());
    if (!ActivationKeyString.empty())
        mActivationKey = cKeyboardServer::ConfigStringToEventID(ActivationKeyString);
}

void cPushButton::Init(const cInitData &InitData)
{
	cDialogItem::Init(InitData);
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	if(InitData.mActivationKey!=0)
	{
		mKeyboardListeningID=cKeyboardServer::Get().GetDispatcher(cKeyboardServer::Keyboard_KeyDown_First+InitData.mActivationKey)->RegisterListener(
			[this](auto &Event)
		{
			if(IsEnabled()&&thePixieDesktop.IsReachable(GetWindow()))
				mEventDispatchers.PostEvent(Event_Activated);
		});
	}
	InitVisualizer(InitData, InitData.mVisualizer);
	if(!InitData.mTitle.empty())
		SetText(InitData.mTitle);
}

void cPushButton::OnMouseMove(cPoint ScreenCoords, bool IsInside)
{
	cDialogItem::OnMouseMove(ScreenCoords, IsInside);
	if(!IsEnabled())
		return;
	bool WasStateChanged=false;
	if(IsInside&&!mIsMouseInside)
	{
		WasStateChanged=true;
	}
	else
	{
		if(!IsInside&&mIsMouseInside)
		{
			WasStateChanged=true;
		}
	}
	mIsMouseInside=IsInside;
	if(WasStateChanged&&mVisualizer)
		mVisualizer->StateChanged();
}

void cPushButton::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	mIsMouseButtonDown=true;
	if(IsInside)
	{
		StartMouseTracking();
		mVisualizer->StateChanged();
	}
}

void cPushButton::OnLeftButtonUp(cPoint ScreenCoords, bool IsInside)
{
	if(!IsEnabled())
		return;
	if(IsMouseTrackingActive())
	{
		if(IsInside)
		{
			mEventDispatchers.PostEvent(Event_Activated);
		}
		StopMouseTracking();
	}
	mIsMouseButtonDown=false;
	mVisualizer->StateChanged();
}

void cPushButton::DialogItemStateChanged()
{
	if(!IsEnabled())
	{
		mIsMouseInside=false;
		mIsMouseButtonDown=false;
		StopMouseTracking();
	}
	mVisualizer->StateChanged();
}
