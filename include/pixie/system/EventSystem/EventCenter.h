#pragma once

class cEventCenter: public tSingleton<cEventCenter>
{
	typedef std::vector<std::tuple<cEvent, tIntrusivePtr<cEventDispatcher>>> cEvents;
	cEvents mEvents_Reading, mEvents_Writing;
public:
	cEventCenter();

	void DispatchEvents();

	void PostEvent(cEvent &&Event, tIntrusivePtr<cEventDispatcher> Dispatcher);
};

extern cEventCenter *theEventCenter;
