#pragma once

class cMouseServer: public tSingleton<cMouseServer>
{
public:
	enum eMouseEvent
	{
		Event_Move = PixieEvents::MouseServer_First,
		Event_LeftButtonDown, Event_LeftButtonUp, Event_LeftButtonDoubleClick, Event_RightButtonDown, Event_RightButtonUp, Event_Wheel
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
	cEventDispatchers mEventDispatchers;

	enum { DefaultOrder = 100 };
	cRegisteredIDList mListenerIDs;
private:
	cPoint mMousePosition;
	tDataHolder<cPoint> &mMousePositionHolder=tDataHolder<cPoint>::Get();
	tDataHolder<int> &mWheelDeltaHolder=tDataHolder<int>::Get();
	void PostEvent(eMouseEvent Event);
	cWindowsMessageResult OnMouseMove(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnLButtonDown(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnLButtonUp(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnRButtonDown(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnRButtonUp(WPARAM wParam, LPARAM lParam);
	cWindowsMessageResult OnWheel(WPARAM wParam, LPARAM lParam);
public:
	cMouseServer();
	const cPoint &GetMousePosition() const { return mMousePosition; }
	tIntrusivePtr<cEventDispatcher> GetDispatcher(size_t EventIndex) const { return mEventDispatchers[EventIndex]; }
};