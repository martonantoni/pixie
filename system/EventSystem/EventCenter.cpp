#include "StdAfx.h"

cEventCenter *theEventCenter=NULL;

cEventCenter::cEventCenter()
{
	theEventCenter=this;
}

void cEventCenter::PostEvent(cEvent &&Event, tIntrusivePtr<cEventDispatcher> Dispatcher)
{
	mEvents_Writing.emplace_back(std::move(Event), std::move(Dispatcher));
}

void cEventCenter::DispatchEvents()
{
	std::swap(mEvents_Reading, mEvents_Writing);
	for(auto &Event: mEvents_Reading)
	{
		std::get<1>(Event)->Internal_DispatchEvent(std::get<0>(Event));
	}
	mEvents_Reading.clear();
}
