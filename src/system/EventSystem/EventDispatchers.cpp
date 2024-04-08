#include "StdAfx.h"

void cEventDispatchers::Init(const tIntrusivePtr<cEventDispatcher> &RootDispatcher)
{
	if(RootDispatcher)
		mRootDispatcher=RootDispatcher;
	else
		mRootDispatcher.reset(new cEventDispatcher);
}

void cEventDispatchers::Init(const cResourceLocation &Location)
{
	if(Location.empty())
	{
		mRootDispatcher.reset(new cEventDispatcher);
	}
	else
	{
		mRootDispatcher=cEventDispatcher::GetGlobalDispatcher(Location);
	}
}

void cEventDispatchers::AddEvents(const cDispatcherRangeInfo &RangeInfo)
{
	ASSERT(mRootDispatcher);
	mRanges.emplace_back(RangeInfo);	
}

void cEventDispatchers::addEvents(const cEventNames& eventNames)
{
    ASSERT(mRootDispatcher);
	AddEvents(cDispatcherRangeInfo(0, eventNames));
}

void cEventDispatchers::PostEvent(size_t DispatcherIndex, cEvent &&Event)
{
	const auto &Dispatcher=(*this)[DispatcherIndex];
	if(!Dispatcher)
		return;
	Dispatcher->PostEvent(std::move(Event));
}

tIntrusivePtr<cEventDispatcher> cEventDispatchers::operator[](size_t Index) const
{
	auto range_i=std::ranges::find_if(mRanges, [Index](auto &Range) 
		{ return Range.mInfo.mFirstIndex<=Index&&(Range.mInfo.mFirstIndex+Range.mInfo.mEventNames.size())>Index; });
	if(ASSERTFALSE(range_i==mRanges.end()))
		return nullptr;
	size_t WithinRangeIndex=Index-range_i->mInfo.mFirstIndex;
	auto &Dispatcher=range_i->mDispatchers[WithinRangeIndex];
	if(!Dispatcher)
	{
		const_cast<tIntrusivePtr<cEventDispatcher>&>(Dispatcher)=   // late init pattern, const_cast is allowed
			(mRootDispatcher->GetSubResource(range_i->mInfo.mEventNames[WithinRangeIndex], cEventDispatcher::CanCreate));
	}
	return Dispatcher;
}
