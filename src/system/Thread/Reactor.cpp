#include "StdAfx.h"

cReactor::cReactor()
{
}

cReactor::~cReactor()
{
	ASSERT(mEventListeners.empty());
	ASSERT(mHandles.empty());
	ASSERT(mIDs.empty());
}

size_t cReactor::AddEventListener(const cEventListener &EventListener,cNativeEvent *Event)
{
	mAddOrRemoveTrap=true;
	mEventListeners.push_back(EventListener);
	mHandles.push_back(Event->GetHandle());
	mIDs.push_back(++mIDCounter);
	return mIDCounter;
}

void cReactor::SwapListeners(size_t First, size_t Second)
{
	std::swap(mEventListeners[First],mEventListeners[Second]);
	std::swap(mHandles[First],mHandles[Second]);
	std::swap(mIDs[First],mIDs[Second]);
}

void cReactor::RemoveEventListener(size_t ID)
{
	auto i=std::find(mIDs, ID);
	if(i==mIDs.end())
		return;
	mAddOrRemoveTrap=true;
	size_t Index=i-mIDs.begin(),LastIndex=mIDs.size()-1;
	if(Index!=LastIndex)
	{
		SwapListeners(Index, LastIndex);
	}
	mEventListeners.resize(LastIndex);
	mHandles.resize(LastIndex);
	mIDs.resize(LastIndex);
}

DWORD cReactor::Wait(DWORD NumberOfHandles,DWORD Timeout)
{
	return ::WaitForMultipleObjects(NumberOfHandles,&mHandles[0],false,Timeout);
}

bool cReactor::DispatchEvents(DWORD Timeout)
{
	if(mEventListeners.empty())
	{
		::Sleep(Timeout);
		return false;
	}
	mAddOrRemoveTrap=false;
	DWORD NumberOfEvents=(DWORD)mEventListeners.size();
	bool WasEventDispatched=false;
	for(;;)
	{
		DWORD WaitResult=Wait(NumberOfEvents,Timeout);
		ASSERT(WaitResult!=WAIT_FAILED);
		if(WaitResult==WAIT_TIMEOUT)
		{
			return WasEventDispatched;
		}
		DWORD EventIndex=WaitResult-WAIT_OBJECT_0;
		ASSERT(EventIndex<mEventListeners.size());
		mEventListeners[EventIndex]();
		if(mAddOrRemoveTrap)
			return true;
		WasEventDispatched=true;
		Timeout=0;
		if(!--NumberOfEvents)
			return true;
		SwapListeners(EventIndex, NumberOfEvents);
	} 
}
