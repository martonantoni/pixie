#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "SpriteRenderer2.h"

cPixieDesktop thePixieDesktop;

cPixieDesktop::cPixieDesktop()
{
}

void cPixieDesktop::Init(const cInitData &InitData)
{
	if(ASSERTFALSE(mRenderer.get()))
		return;
	cPixieWindow::Init(InitData);
	SetPlacement(cRect(0, 0, theDevice->GetPresentParameters().BackBufferWidth, theDevice->GetPresentParameters().BackBufferHeight));

	mRenderer=std::make_unique<cSpriteRenderer>(*this);
	cDevice::Get()->AddMainRenderer(mRenderer.get());

	cPixieWindow::cInitData TopLayerInit;
	TopLayerInit.mZOrder=1'000'000;
	TopLayerInit.mPlacement=GetPlacement();
	TopLayerInit.mParentWindow=this;
	TopLayerInit.mFlags=Flag_ClickThrough;
	mTopLayer.Init(TopLayerInit);

	cMouseServer &MouseServer=cMouseServer::Get();
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_Move)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnMouseMove); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonDown)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnLeftButtonDown); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonUp)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnLeftButtonUp); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonDoubleClick)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnLeftButtonDoubleClick); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_RightButtonDown)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnRightButtonDown); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_RightButtonUp)->
		RegisterListener([this](const auto &Event) { HandleMouseEvent(Event, &cMouseTarget::OnRightButtonUp); }));
}

void cPixieDesktop::HandleMouseEvent(const cEvent &Event, void (cMouseTarget::*MouseEventHandlerFunction)(cPoint Point, bool IsInside))
{
	const cPoint *ScreenCoordinates=mPointHolder.GetData(Event.mEventDataID);
	if(ASSERTFALSE(!ScreenCoordinates))
		return;
	auto TargetResult=GetMouseTargetAt(*ScreenCoordinates);
	cMouseTarget *IgnoredTarget=nullptr;
	if(MouseEventHandlerFunction==&cMouseTarget::OnMouseMove)
	{
		if(mLastMoveTarget!=TargetResult.mResult)
		{
			if(mLastMoveTarget)
			{
				mLastMoveTarget->OnMouseMove(*ScreenCoordinates, false);
				IgnoredTarget=mLastMoveTarget;
			}
			mLastMoveTarget=TargetResult.mResult;
		}
	}
	mMouseTrackers.ForEach([&](auto MouseTarget)
	{
		if(TargetResult.mResult!=MouseTarget&&(!IgnoredTarget||TargetResult.mResult!=IgnoredTarget))
			(MouseTarget->*MouseEventHandlerFunction)(*ScreenCoordinates,false);
	});
	if(TargetResult.mResult)
		(TargetResult.mResult->*MouseEventHandlerFunction)(*ScreenCoordinates,true);
}

void cPixieDesktop::MouseTargetRemoved(cMouseTarget *MouseTarget)
{
	if(mLastMoveTarget==MouseTarget)
		mLastMoveTarget=nullptr;
}

cRegisteredID cPixieDesktop::RegisterMouseTracker(cMouseTarget *MouseTarget)
{
	return mMouseTrackers.Register(MouseTarget);
}

bool cPixieDesktop::IsReachable(const cPixieWindow *Window) const
{
	switch(CheckModality(Window))
	{
	case cPixieWindow::eModalityCheckResult::Blocked:
		return false;
	case cPixieWindow::eModalityCheckResult::Reached:
		return true;
	case cPixieWindow::eModalityCheckResult::NotFound:
		return Window==this;
	}
	return false;
}
