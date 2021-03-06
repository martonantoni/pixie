#include "StdAfx.h"

const cEventDispatchers::cDispatcherRangeInfo cMouseServer::mDispatcherRangeInfo=
{
	PixieEvents::MouseServer_First, { "move", "left_button_down", "left_button_up", "left_button_double_click", "right_button_down", "right_button_up", "wheel" }
};


cMouseServer::cMouseServer()
	: mMousePosition(0, 0)
{
	cPrimaryWindow &PrimaryWindow=cPrimaryWindow::Get();
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_MOUSEMOVE, [this](auto wp, auto lp) { return OnMouseMove(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_LBUTTONDOWN, [this](auto wp, auto lp) {return OnLButtonDown(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_LBUTTONDBLCLK, [this](auto wp, auto lp) {return OnLButtonDoubleClick(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_LBUTTONUP, [this](auto wp, auto lp) {return OnLButtonUp(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_RBUTTONDOWN, [this](auto wp, auto lp) {return OnRButtonDown(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_RBUTTONUP, [this](auto wp, auto lp) {return OnRButtonUp(wp, lp); }));
	mListenerIDs.push_back(PrimaryWindow.AddMessageHandler(WM_MOUSEWHEEL, [this](auto wp, auto lp) {return OnWheel(wp, lp); }));

	mEventDispatchers.Init(cEventDispatcher::GetGlobalDispatcher("pixie.mouse", cEventDispatcher::CanCreate));
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	cEventDispatcher::cEventListenerRequest ListenerRequest;
	ListenerRequest.mOrder=0;
}

void cMouseServer::PostEvent(eMouseEvent Event)
{
	mEventDispatchers.PostEvent(Event, cEvent(mMousePositionHolder.StoreData(mMousePosition)));
}

cWindowsMessageResult cMouseServer::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_Move);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::SetCapture(cPrimaryWindow::Get().mWindowHandle);
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_LeftButtonDown);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	::ReleaseCapture();
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_LeftButtonUp);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam)
{
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_LeftButtonDoubleClick);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_RightButtonDown);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	mMousePosition=cPoint(LOWORD(lParam), HIWORD(lParam));
	PostEvent(Event_RightButtonUp);
	return cWindowsMessageResult();
}

cWindowsMessageResult cMouseServer::OnWheel(WPARAM wParam, LPARAM lParam)
{
	mEventDispatchers.PostEvent(Event_Wheel, cEvent(mWheelDeltaHolder.StoreData(GET_WHEEL_DELTA_WPARAM(wParam))));
	return cWindowsMessageResult();
}

