#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
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
	switch (mClippingMode)
	{
	case eClippingMode::None:
		break;
	case eClippingMode::Parent:
		if (!mValidRect.isPointInside(WindowRelativePoint))
			return false;
		break;
	case eClippingMode::ParentParent:
	{
		auto screenValidRect = mValidRect;
		screenValidRect.position() += mWindow->GetParentWindow()->GetScreenRect().position();
		auto screenPoint = mWindow->WindowCoordinatesToScreenCoordinates(WindowRelativePoint);
		if (!screenValidRect.isPointInside(screenPoint))
            return false;
		break;
	}
	case eClippingMode::Screen:
		if (!mValidRect.isPointInside(mWindow->WindowCoordinatesToScreenCoordinates(WindowRelativePoint)))
			return false;
		break;
	}
	if(mIgnoreBoundingBoxForIsInside)
	{
		return IsInside_Overridable(WindowRelativePoint);
	}
	return mWindowRelativeBoundingBox.isPointInside(WindowRelativePoint)&&IsInside_Overridable(WindowRelativePoint);
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
		Rect.position() += mWindow->GetScreenRect().position();
	return Rect;
}

void cMouseTarget::SetPosition(cPoint Position)
{
	SetPlacement({ Position, mWindowRelativeBoundingBox.size() });
}

void cMouseTarget::SetValidRect(const cRect &ValidRect)
{
	mValidRect = ValidRect;
	if (ValidRect.width() >= 0 && ValidRect.height() >= 0 && mClippingMode == eClippingMode::None) // backward compatibility
	{
		mClippingMode = eClippingMode::Parent;
		PropertiesSet(Property_ClippingMode);
	}
	PropertiesSet(Property_ValidRect);
}

void cMouseTarget::setClippingMode(eClippingMode ClippingMode)
{
	mClippingMode = ClippingMode;
	PropertiesSet(Property_ClippingMode);
}

cPoint cMouseTarget::ScreenCoordsToWindowRelativeCoords(cPoint ScreenCoords) const
{
	if(ASSERTFALSE(!mWindow))
		return cPoint();
	return ScreenCoords-mWindow->GetScreenRect().position();
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
	case Property_X: PropertyValues=mWindowRelativeBoundingBox.left(); return true;
	case Property_Y: PropertyValues=mWindowRelativeBoundingBox.top(); return true;
	case Property_W: PropertyValues=mWindowRelativeBoundingBox.width(); return true;
	case Property_H: PropertyValues=mWindowRelativeBoundingBox.height(); return true;
	case Property_ZOrder: PropertyValues=GetZOrder(); return true;
	case Property_Position: PropertyValues=mWindowRelativeBoundingBox.position(); return true;
	case Property_Size: PropertyValues=mWindowRelativeBoundingBox.size(); return true;
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
	case Property_X: SetPlacement({ Value.ToInt(), mWindowRelativeBoundingBox.top(), mWindowRelativeBoundingBox.width(), mWindowRelativeBoundingBox.height() }); return true;
	case Property_Y: SetPlacement({ mWindowRelativeBoundingBox.left(), Value.ToInt(), mWindowRelativeBoundingBox.width(), mWindowRelativeBoundingBox.height() }); return true;
	case Property_W: SetPlacement({ mWindowRelativeBoundingBox.left(), mWindowRelativeBoundingBox.top(), Value.ToInt(), mWindowRelativeBoundingBox.height() }); return true;
	case Property_H: SetPlacement({ mWindowRelativeBoundingBox.left(), mWindowRelativeBoundingBox.top(), mWindowRelativeBoundingBox.width(), Value.ToInt() }); return true;
//	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Position: SetPlacement({ Value.ToPoint(), mWindowRelativeBoundingBox.size() }); return true;
	case Property_Size: SetPlacement({ mWindowRelativeBoundingBox.position(), Value.ToPoint() }); return true;
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

