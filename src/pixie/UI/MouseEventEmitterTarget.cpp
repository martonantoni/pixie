#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
const cEventDispatchers::cDispatcherRangeInfo cMouseEventEmitterTarget::mDispatcherRangeInfo=
{
	PixieEvents::EventEmitterTarget_First,
	{
		"move_inside", "move_outside",
		"left_button_down", "left_button_up",
		"right_button_down", "right_button_up",
		"left_button_double_click"
	}
};

void cMouseEventEmitterTarget::Init(const cInitData &InitData)
{
	cMouseTarget::Init(InitData);
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
}

void cMouseEventEmitterTarget::OnMouseMove(cPoint ScreenCoords, bool IsInside)
{
	if(IsInside==mWasLastMoveInside)
		return;
	mWasLastMoveInside=IsInside;
	if(IsInside)
	{
		mEventDispatchers.PostEvent(Event_MoveInside);
	}
	else
	{
		mEventDispatchers.PostEvent(Event_MoveOutside);
	}
}

void cMouseEventEmitterTarget::OnLeftButtonDown(cPoint ScreenCoords, bool IsInside)
{
	mEventDispatchers.PostEvent(Event_LeftButtonDown);
}

void cMouseEventEmitterTarget::OnLeftButtonUp(cPoint ScreenCoords, bool IsInside)
{
	mEventDispatchers.PostEvent(Event_LeftButtonUp);
}

void cMouseEventEmitterTarget::OnLeftButtonDoubleClick(cPoint ScreenCoords, bool IsInside)
{
	mEventDispatchers.PostEvent(Event_LeftButtonDoubleClick);
}

void cMouseEventEmitterTarget::OnRightButtonDown(cPoint ScreenCoords, bool IsInside)
{
	mEventDispatchers.PostEvent(Event_RightButtonDown);
}

void cMouseEventEmitterTarget::OnRightButtonUp(cPoint ScreenCoords, bool IsInside)
{
	mEventDispatchers.PostEvent(Event_RightButtonUp);
}

std::unique_ptr<cMouseEventEmitterTarget> cMouseEventEmitterTarget::CreateOverSprite(const cSpriteBase& sprite)
{
    cMouseEventEmitterTarget::cInitData TargetInitData;
    TargetInitData.mZOrder = sprite.GetZOrder(); // doesn't make much sense, but why not
    TargetInitData.mParentWindow = sprite.GetWindow();
    TargetInitData.mPlacement = sprite.GetRect();
    auto target = std::make_unique<cMouseEventEmitterTarget>();
    target->Init(TargetInitData);
    return target;
}
