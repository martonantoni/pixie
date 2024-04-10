#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/2dEngine/sprite_renderer.h"

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
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnMouseMove); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonDown)->
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnLeftButtonDown); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonUp)->
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnLeftButtonUp); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonDoubleClick)->
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnLeftButtonDoubleClick); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_RightButtonDown)->
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnRightButtonDown); }));
	mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_RightButtonUp)->
		RegisterListener([this](const auto &event) { handleMouseEvent(event, &cMouseTarget::OnRightButtonUp); }));
    mMouseEventListeners.push_back(MouseServer.GetDispatcher(cMouseServer::Event_Wheel)->
        RegisterListener([this](const auto& event) { handleMouseWheel(event); }));
}

void cPixieDesktop::handleMouseEvent(const cEvent& event, void (cMouseTarget::*MouseEventHandlerFunction)(cPoint Point, bool IsInside))
{
	mMouseEventShiftState = cMouseServer::shiftState(event);
	mMouseEventCtrlState = cMouseServer::ctrlState(event);
	cPoint screenCoordinates = cMouseServer::point(event);
	auto TargetResult=GetMouseTargetAt(screenCoordinates);
	cMouseTarget *IgnoredTarget=nullptr;
	if(MouseEventHandlerFunction==&cMouseTarget::OnMouseMove)
	{
		if(mLastMoveTarget!=TargetResult.mResult)
		{
			if(mLastMoveTarget)
			{
				mLastMoveTarget->OnMouseMove(screenCoordinates, false);
				IgnoredTarget=mLastMoveTarget;
			}
			mLastMoveTarget=TargetResult.mResult;
		}
	}
	mMouseTrackers.ForEach([&](auto MouseTarget)
	{
		if(TargetResult.mResult!=MouseTarget&&(!IgnoredTarget||TargetResult.mResult!=IgnoredTarget))
			(MouseTarget->*MouseEventHandlerFunction)(screenCoordinates,false);
	});
	if(TargetResult.mResult)
		(TargetResult.mResult->*MouseEventHandlerFunction)(screenCoordinates,true);
}

void cPixieDesktop::handleMouseWheel(const cEvent& event)
{
    mMouseEventShiftState = cMouseServer::shiftState(event);
    mMouseEventCtrlState = cMouseServer::ctrlState(event);
    cPoint screenCoordinates = cMouseServer::point(event);
	double wheelDelta = cMouseServer::wheelDelta(event);
    auto TargetResult = GetMouseTargetAt(screenCoordinates);
	if (TargetResult.mResult)
		TargetResult.mResult->OnMouseWheel(screenCoordinates, wheelDelta, true);
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
