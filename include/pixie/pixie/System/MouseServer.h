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
	struct cEventData
	{
		union
		{
			struct
			{
				int64_t x : 20;
				int64_t y : 20;
				int64_t wheelDelta : 20;
				int64_t altState : 1;
				int64_t shiftState : 1;
				int64_t ctrlState : 1;
			} parts;
			uint64_t packed;
		};
	};
    static_assert(sizeof(cEventData) == sizeof(uint64_t));
	enum { DefaultOrder = 100 };
	cRegisteredIDList mListenerIDs;
private:
	cPoint mMousePosition;
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

	static cPoint point(const cEvent& event);
	static double wheelDelta(const cEvent& event);
	static bool shiftState(const cEvent& event);
	static bool ctrlState(const cEvent& event);
	static bool altState(const cEvent& event);
};