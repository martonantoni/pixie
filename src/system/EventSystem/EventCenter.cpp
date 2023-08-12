#include "StdAfx.h"

cEventCenter *theEventCenter=NULL;

cEventCenter::cEventCenter()
{
	theEventCenter=this;
}

void cEventCenter::PostEvent(cEvent &&Event, tIntrusivePtr<cEventDispatcher> Dispatcher)
{
	mEvents_Writing.emplace_back(std::move(Event), std::move(Dispatcher));
	if (!mDispatchingEvents && mEvents_Writing.size() == 1 && mNeedDispatchProcessor)
	{
		mNeedDispatchProcessor();
	}
}

void cEventCenter::DispatchEvents(DWORD loopUntil)
{
    mDispatchingEvents = true;
	for (;;)
	{
		std::swap(mEvents_Reading, mEvents_Writing);
		for (auto& Event : mEvents_Reading)
		{
			std::get<1>(Event)->Internal_DispatchEvent(std::get<0>(Event));
		}
		mEvents_Reading.clear();
		if (loopUntil == 0)
			break;
		if (mEvents_Writing.empty())
			break;
		if (loopUntil - ::GetTickCount() > 0xf000'0000u)
			break;
	}
	mDispatchingEvents = false;
    if (!mEvents_Writing.empty() && mNeedDispatchProcessor)
    {
        mNeedDispatchProcessor();
    }
}
