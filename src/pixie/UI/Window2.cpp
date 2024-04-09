#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/Window2_SpriteIterator.h"

const cEventDispatchers::cDispatcherRangeInfo cPixieWindow::mDispatcherRangeInfo
{
	PixieEvents::Window_First, { "placement_changed", "visibility_changed", "parent_window_changed" }
};

extern bool g_IsExitActive;

cPixieWindow::~cPixieWindow()
{
	if(!g_IsExitActive) //todo fix this with ProgramDirector, and properly killing objects
	{
		while(!mSprites.empty())
			mSprites.back()->SetWindow(nullptr);
		while(!mMouseTargets.empty())
			mMouseTargets.back()->SetWindow(nullptr);
		while(!mSubWindows.empty())
			mSubWindows.back()->SetParentWindow(nullptr);
		if(mParentWindow)
			mParentWindow->RemoveSubWindow(this);
	}
}

void cPixieWindow::Init(const cInitData &InitData)
{
// if you are in this function, and not in a subclass' Init, that's because you used the wrong cInitData type
	mEventDispatchers.Init(InitData.mBindTo);
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	mEventDispatchers[Event_ParentWindowChanged]->SetCoalescing();
	mEventDispatchers[Event_PlacementChanged]->SetCoalescing();
	mEventDispatchers[Event_VisibilityChanged]->SetCoalescing();
	mZOrder=InitData.mZOrder;
	mParentWindow=InitData.mParentWindow;
	if(mParentWindow)
		mParentWindow->AddSubWindow(this);
	if(!(InitData.mFlags&Flag_ClickThrough))
	{
		cInitData MouseBlockerInit;
		MouseBlockerInit.mFlags=cMouseTarget::Flag_IgnoreBoundingBoxForIsInside;
		MouseBlockerInit.mParentWindow=this;
		MouseBlockerInit.mZOrder=0;
		mMouseBlocker=std::make_unique<cMouseBlocker>();
		mMouseBlocker->Init(MouseBlockerInit);
	}
	else
		mIsClickThrough=true;
	mIsModal=InitData.mFlags&Flag_IsModal;
	SetPlacement(InitData.mPlacement); // uses mEventDispatchers & mParentWindow
}

tIntrusivePtr<cEventDispatcher> cPixieWindow::GetDispatcher(eEventType EventType) const
{
	return mEventDispatchers[EventType];
}

void cPixieWindow::AddSprite(cSpriteBase *Sprite)
{
	auto i=std::lower_bound(ALL(mSprites), Sprite->GetZOrder(), [](auto &Sprite, auto ZOrder) { return Sprite->GetZOrder()<ZOrder; });
	mSprites.insert(i, Sprite);
}

void cPixieWindow::RemoveSprite(cSpriteBase *Sprite)
{
	auto i=std::ranges::find(mSprites, Sprite);
	if(ASSERTTRUE(i!=mSprites.end()))
		mSprites.erase(i);
}

cPixieWindow::cSpriteIterator cPixieWindow::CreateSpriteIterator() const
{
	return cSpriteIterator(*this);
}

void cPixieWindow::AddSubWindow(cPixieWindow *SubWindow)
{
	auto i=std::lower_bound(ALL(mSubWindows), SubWindow->mZOrder, [](auto &Window, auto ZOrder) { return Window->mZOrder>ZOrder; });
	mSubWindows.insert(i,SubWindow);
	SubWindow->UpdateScreenRect(mScreenRect);
}

void cPixieWindow::RemoveSubWindow(cPixieWindow *SubWindow)
{
	auto i=std::ranges::find(mSubWindows, SubWindow);
	if(i!=mSubWindows.end())
		mSubWindows.erase(i);
}

void cPixieWindow::SetParentWindow(cPixieWindow *ParentWindow)
{
	if(mParentWindow)
		mParentWindow->RemoveSubWindow(this);
	mParentWindow=ParentWindow;
	if(mParentWindow)
		mParentWindow->AddSubWindow(this);
	mEventDispatchers.PostEvent(Event_ParentWindowChanged);
}

void cPixieWindow::AddMouseTarget(cMouseTarget *Target)
{
	auto i=std::lower_bound(ALL(mMouseTargets), Target->GetZOrder(), [](auto &Target, auto ZOrder) { return Target->GetZOrder()>ZOrder; });
	mMouseTargets.insert(i, Target);
}

void cPixieWindow::RemoveMouseTarget(cMouseTarget *Target)
{
	auto i=std::ranges::find(mMouseTargets, Target);
	if(i!=mMouseTargets.end())
		mMouseTargets.erase(i);
	thePixieDesktop.MouseTargetRemoved(Target);
}

void cPixieWindow::UpdateScreenRect(const cRect &ParentScreenRect)
{
	mScreenRect=mPlacement;
	mScreenRect.Move(ParentScreenRect.GetPosition());
	for(auto &SubWindow: mSubWindows)
		SubWindow->UpdateScreenRect(mScreenRect);
}

cPoint cPixieWindow::WindowCoordinatesToScreenCoordinates(cPoint WindowRelativeCoords) const
{
	return GetScreenRect().GetPosition()+WindowRelativeCoords;
}

cPoint cPixieWindow::ScreenCoordinatesToWindowCoordinates(cPoint ScreenCoords) const
{
	return ScreenCoords-GetScreenRect().GetPosition();
}

void cPixieWindow::SetPlacement(const cRect &Rect)
{
	mPlacement=Rect;
	UpdateScreenRect(mParentWindow?mParentWindow->GetScreenRect():cRect(0, 0, 1, 1));
	PropertiesSet(Property_Rect);
	mEventDispatchers.PostEvent(Event_PlacementChanged);
}

void cPixieWindow::setPosition(const cPoint& Position)
{
	SetPlacement({ Position, mPlacement.GetSize() });
}

void cPixieWindow::setSize(const cPoint& Size)
{
    SetPlacement({ mPlacement.GetPosition(), Size });
}

bool cPixieWindow::IsInside(cPoint WindowRelativeCoords) const
{
	return mPlacement.IsPointInside(WindowRelativeCoords); //todo: check visualizer! (this will be a virtual call)
}

cPixieWindow::tGetAtResult<cMouseTarget> cPixieWindow::GetMouseTargetAt(cPoint Point) const
{
	for(auto &SubWindow: mSubWindows)
	{
		auto Result=SubWindow->GetMouseTargetAt(Point-SubWindow->GetPlacement().GetPosition());
		if(Result.mResult)
			return Result;
 		if(SubWindow->mIsModal)
 			return tGetAtResult<cMouseTarget>();
	}
	for(auto &Target: mMouseTargets)
	{
		if(Target->IsInside(Point))
			return tGetAtResult<cMouseTarget>(Target, Point);
	}
	return tGetAtResult<cMouseTarget>();
}

cPixieWindow::eModalityCheckResult cPixieWindow::CheckModality(const cPixieWindow *WindowToReach) const
{
	for(auto &SubWindow: mSubWindows)
	{
		auto Result=SubWindow->CheckModality(WindowToReach);
		if(Result!=eModalityCheckResult::NotFound)
			return Result;
		if(SubWindow==WindowToReach)
			return eModalityCheckResult::Reached;
		if(SubWindow->mIsModal)
			return eModalityCheckResult::Blocked;
	}
	return eModalityCheckResult::NotFound;
}

cPixieWindow::tGetAtResult<cPixieWindow> cPixieWindow::GetWindowAt(cPoint Point, eGetWindowRules GetWindowRules) const
{
	for(auto &SubWindow: mSubWindows)
	{
		if(GetWindowRules==GetWindow_ClickRules&&SubWindow->mIsClickThrough)
			continue;
		if(SubWindow->GetPlacement().IsPointInside(Point))
		{
			auto Result=SubWindow->GetWindowAt(Point-SubWindow->GetPlacement().GetPosition(), GetWindowRules);
			if(Result.mResult)
				return Result;
			return tGetAtResult<cPixieWindow>(SubWindow, Point);
		}
	}
	return tGetAtResult<cPixieWindow>();
}

void cPixieWindow::CheckOwnerlessSprites()
{
	for(auto &SubWindow: mSubWindows)
	{
		SubWindow->CheckOwnerlessSprites();
	}
	for(;;)
	{
		bool WasKill=false;
		std::erase_if(mSprites, [&WasKill](auto Sprite)
		{
			if(Sprite->DestroyZombie()==cSpriteBase::eDestroyZombieResult::Destroyed)
			{
				WasKill=true;
				return true;
			}
			return false;
		});
		if(!WasKill)
			break;
	}
}

bool cPixieWindow::GetProperty(unsigned int PropertyFlags, OUT cPropertyValues &PropertyValues) const
{
	switch(PropertyFlags)
	{
	case Property_X: PropertyValues=mPlacement.mLeft; return true;
	case Property_Y: PropertyValues=mPlacement.mTop; return true;
	case Property_W: PropertyValues=mPlacement.mWidth; return true;
	case Property_H: PropertyValues=mPlacement.mHeight; return true;
	case Property_ZOrder: PropertyValues=GetZOrder(); return true;
	case Property_Position: PropertyValues=mPlacement.GetPosition(); return true;
	case Property_Size: PropertyValues=mPlacement.GetSize(); return true;
	case Property_Rect: PropertyValues=mPlacement; return true;
	}
	ASSERT(false);
	return false;
}

bool cPixieWindow::SetProperty(unsigned int PropertyFlags, const cPropertyValues &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	switch(PropertyFlags)
	{
	case Property_X: SetPlacement({ Value.ToInt(), mPlacement.mTop, mPlacement.mWidth, mPlacement.mHeight }); return true;
	case Property_Y: SetPlacement({ mPlacement.mLeft, Value.ToInt(), mPlacement.mWidth, mPlacement.mHeight }); return true;
	case Property_W: SetPlacement({ mPlacement.mLeft, mPlacement.mTop, Value.ToInt(), mPlacement.mHeight }); return true;
	case Property_H: SetPlacement({ mPlacement.mLeft, mPlacement.mTop, mPlacement.mWidth, Value.ToInt() }); return true;
		//	case Property_ZOrder: SetZOrder(Value.ToInt()); return true;
	case Property_Position: SetPlacement({ Value.ToPoint(), mPlacement.GetSize() }); return true;
	case Property_Size: SetPlacement({ mPlacement.GetPosition(), Value.ToPoint() }); return true;
	case Property_Rect: SetPlacement(Value.ToRect()); return true;
	}
	ASSERT(false);
	return false;
}

bool cPixieWindow::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	ASSERT(false);
	return false;
}

bool cMouseBlocker::IsInside_Overridable(cPoint WindowRelativePoint) const
{
	return GetWindow()->IsInside(WindowRelativePoint+GetWindow()->GetPlacement().GetPosition()); // window's placement is relative to its parent window
}
