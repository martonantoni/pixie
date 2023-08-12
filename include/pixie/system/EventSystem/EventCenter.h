#pragma once

class cEventCenter: public tSingleton<cEventCenter>
{
	typedef std::vector<std::tuple<cEvent, tIntrusivePtr<cEventDispatcher>>> cEvents;
	cEvents mEvents_Reading, mEvents_Writing;
	bool mDispatchingEvents = false;
	DWORD mTimeToNextTimer;
	std::function<void()> mNeedDispatchProcessor;
public:
	cEventCenter();

	void DispatchEvents(DWORD loopUntil = 0);

	void PostEvent(cEvent &&Event, tIntrusivePtr<cEventDispatcher> Dispatcher);
	void setNeedDispatchProcessor(const std::function<void()>& processor)
	{
		mNeedDispatchProcessor = processor;
	}
};

extern cEventCenter *theEventCenter;
