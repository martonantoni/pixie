#include "StdAfx.h"

const cEventDispatchers::cDispatcherRangeInfo cMouseTarget::mDispatcherRangeInfo
{
	PixieEvents::MouseTarget_First, { "placement_changed", "parent_window_changed" }
};

unsigned int cMouseTarget::mMouseTargetIDCounter=0;

cMouseTarget::~cMouseTarget()
{
	if(mWindow)
		mWindow->RemoveMouseTarget(this);
}

bool cMouseTarget::IsInside(cPoint WindowRelativePoint) const
{
	if(mUseClipping&&!mValidRect.IsPointInside(WindowRelativePoint))
		return false;
	if(mIgnoreBoundingBoxForIsInside)
	{
		return IsInside_Overridable(WindowRelativePoint);
	}
	return mWindowRelativeBoundingBox.IsPointInside(WindowRelativePoint)&&IsInside_Overridable(WindowRelativePoint);
}

void cMouseTarget::Init(const cInitData &InitData)
{
	mWindow=InitData.mParentWindow;
	mEventDispatchers.Init(InitData.mBindTo);
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	mEventDispatchers[Event_PlacementChanged]->SetCoalescing();
	mEventDispatchers[Event_ParentWindowChanged]->SetCoalescing();
	mZOrder=InitData.mZOrder;
	mIgnoreBoundingBoxForIsInside=InitData.mFlags&Flag_IgnoreBoundingBoxForIsInside;
	SetPlacement(InitData.mPlacement); // uses mEventDispatchers & mParentWindow
	if(InitData.mParentWindow)
		InitData.mParentWindow->AddMouseTarget(this);
}

void cMouseTarget::SetWindow(cPixieWindow *Window)
{
	if(Window==mWindow)
		return;
	if(mWindow)
		mWindow->RemoveMouseTarget(this);
	mWindow=Window;
	if(mWindow)
		mWindow->AddMouseTarget(this);
	PropertiesSet(Property_Window);
	mEventDispatchers.PostEvent(Event_ParentWindowChanged);
}

void cMouseTarget::SetPlacement(const cRect &Rect)
{
	if(mWindowRelativeBoundingBox==Rect)
		return;
	mWindowRelativeBoundingBox=Rect;
	PropertiesSet(Property_Rect);
	if(mWindow)
	{
		bool IsInside=thePixieDesktop.GetMouseTargetAt(cMouseServer::Get().GetMousePosition()).mResult==this;
		if(IsInside||IsMouseTrackingActive())
			OnMouseMove(cMouseServer::Get().GetMousePosition(), IsInside);
	}
	mEventDispatchers.PostEvent(Event_PlacementChanged);
}

cRect cMouseTarget::GetScreenPlacement() const
{
	auto Rect=GetPlacement();
	if(mWindow)
		Rect.Move(mWindow->GetScreenRect().GetPosition());
	return Rect;
}

void cMouseTarget::SetPosition(cPoint Position)
{
	SetPlacement({ Position, mWindowRelativeBoundingBox.GetSize() });
}

void cMouseTarget::SetValidRect(const cRect &ValidRect)
{
	mUseClipping=true;
	mValidRect=ValidRect;
}

cPoint cMouseTarget::ScreenCoordsToWindowRelativeCoords(cPoint ScreenCoords) const
{
	if(ASSERTFALSE(!mWindow))
		return cPoint();
	return ScreenCoords-mWindow->GetScreenRect().GetPosition();
}

void cMouseTarget::StartMouseTracking()
{
	if(IsMouseTrackingActive())
		return; 
	mMouseTrackingID=thePixieDesktop.RegisterMouseTracker(this);
}

void cMouseTarget::StopMouseTracking()
{
	mMouseTrackingID.Unregister();
}

bool cMouseTarget::IsMouseTrackingActive() const
{
	return mMouseTrackingID.IsValid();
}

bool cMouseTarget::GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &PropertyValues) const
{
	switch(PropertyFlags)
	{
	case Property_X: PropertyValues=mWindowRelativeBoundingBox.mLeft; return true;
	case Property_Y: PropertyValues=mWindowRelativeBoundingBox.mTop; return true;
	case Property_W: PropertyValues=mWindowRelativeBoundingBox.mWidth; return true;
	case Property_H: PropertyValues=mWindowRelativeBoundingBox.mHeight; return true;
	case Property_ZOrder: PropertyValues=GetZOrder(); return true;
	case Property_Position: PropertyValues=mWindowRelativeBoundingBox.GetPosition(); return true;
	case Property_Size: PropertyValues=mWindowRelativeBoundingBox.GetSize(); return true;
	case Property_Rect: PropertyValues=mWindowRelativeBoundingBox; return true;
	}
	ASSERT(false);
	return false;
}

bool cMouseTarget::SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch(PropertyFlags)
	{
	case Property_X: SetPlacement({ Value.ToInt(), mWindowRelativeBoundingBox.mTop, mWindowRelativeBoundingBox.mWidth, mWindowRelativeBoundingBox.mHeight }); return true;
	case Property_Y: SetPlacement({ mWindowRelativeBoundingBox.mLeft, Value.ToInt(), mWindowRelativeBoundingBox.mWidth, mWindowRelativeBoundingBox.mHeight }); return true;
	case Property_W: SetPlacement({ mWindowRelativeBoundingBox.mLeft, mWindowRelativeBoundingBox.mTop, Value.ToInt(), mWindowRelativeBoundingBox.mHeight }); return true;
	case Property_H: SetPlacement({ mWindowRelativeBoundingBox.mLeft, mWindowRelativeBoundingBox.mTop, mWindowRelativeBoundingBox.mWidth, Value.ToInt() }); return true;
//	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Position: SetPlacement({ Value.ToPoint(), mWindowRelativeBoundingBox.GetSize() }); return true;
	case Property_Size: SetPlacement({ mWindowRelativeBoundingBox.GetPosition(), Value.ToPoint() }); return true;
	case Property_Rect: SetPlacement(Value.ToRect()); return true;
	}
	ASSERT(false);
	return false;
}

bool cMouseTarget::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	ASSERT(false);
	return false;
}

