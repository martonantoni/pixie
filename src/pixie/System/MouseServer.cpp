#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include <windowsx.h>

const cEventDispatchers::cDispatcherRangeInfo cMouseServer::mDispatcherRangeInfo=
{
	PixieEvents::MouseServer_First, { "move", "left_button_down", "left_button_up", "left_button_double_click", "right_button_down", "right_button_up", "wheel" }
};


cMouseServer::cMouseServer()
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
	cEventData eventData;
	eventData.packed = 0;
	eventData.parts.x = mMousePosition.x;
	eventData.parts.y = mMousePosition.y;
    if (GetKeyState(VK_SHIFT) & 128)
		eventData.parts.shiftState = 1;
    if (GetKeyState(VK_CONTROL) & 0x80)
        eventData.parts.ctrlState = 1;
	if (GetKeyState(VK_MENU) & 128)
		eventData.parts.altState = 1;
	mEventDispatchers.PostEvent(Event, cEvent(cRegisteredID(nullptr, eventData.packed)));
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
    cEventData eventData;
    eventData.packed = 0;
    eventData.parts.x = GET_X_LPARAM(lParam);
    eventData.parts.y = GET_Y_LPARAM(lParam);

	// adjust coordinates to be relative to the window
	POINT pt;
    pt.x = eventData.parts.x;
    pt.y = eventData.parts.y;
    ScreenToClient(cPrimaryWindow::Get().mWindowHandle, &pt);
    eventData.parts.x = pt.x;
    eventData.parts.y = pt.y;

	eventData.parts.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    if (GetKeyState(VK_SHIFT) & 128)
        eventData.parts.shiftState = 1;
    if (GetKeyState(VK_CONTROL) & 0x80)
        eventData.parts.ctrlState = 1;
    if (GetKeyState(VK_MENU) & 128)
        eventData.parts.altState = 1;
	mEventDispatchers.PostEvent(Event_Wheel, cEvent(cRegisteredID(nullptr, eventData.packed)));
	return cWindowsMessageResult();
}

cPoint cMouseServer::point(const cEvent& event)
{
	cEventData eventData;
	eventData.packed = event.mEventDataID.GetID();
	return cPoint{ static_cast<int>(eventData.parts.x), static_cast<int>(eventData.parts.y) };
}

bool cMouseServer::shiftState(const cEvent& event)
{
    cEventData eventData;
    eventData.packed = event.mEventDataID.GetID();
	return eventData.parts.shiftState;
}

bool cMouseServer::ctrlState(const cEvent& event)
{
    cEventData eventData;
    eventData.packed = event.mEventDataID.GetID();
    return eventData.parts.ctrlState;
}

bool cMouseServer::altState(const cEvent& event)
{
    cEventData eventData;
    eventData.packed = event.mEventDataID.GetID();
    return eventData.parts.altState;
}

double cMouseServer::wheelDelta(const cEvent& event)
{
    cEventData eventData;
    eventData.packed = event.mEventDataID.GetID();
    return eventData.parts.wheelDelta / 120.0;
}
