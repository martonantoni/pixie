#include "StdAfx.h"


cTimerServer::cTimerServer()
: mCurrentTimer(NULL)
{
}

cTimerServer::~cTimerServer()
{
	ASSERT(mTimers.empty());
}

void cTimerServer::InsertTimer(cTimerData *TimerData)
{
	for(cTimers::iterator i=mTimers.begin(),iend=mTimers.end();;++i)
	{
		if(i==iend||TimerData->mNextTrigger<i->mNextTrigger)
		{
			mTimers.insert(i,TimerData);
			return;
		}
	}
}

size_t cTimerServer::AddTimer(const cTimerListener &Listener,const cTimerRequest &Request)
{
	cTimerData *TimerData=new cTimerData;
	TimerData->mListener=Listener;
	TimerData->mRequest=Request;
	TimerData->mNextTrigger=GetTickCount()+Request.mInterval;
	TimerData->mID=++mIDCounter;
	InsertTimer(TimerData);
	return mIDCounter;
}

void cTimerServer::RemoveTimer(size_t ID)
{
	if(mCurrentTimer&&ID==mCurrentTimer->mID)
	{
		SAFEDEL(mCurrentTimer);
		return;
	}
	auto i=std::find_if(ALL(mTimers), [ID](auto Timer) { return Timer->mID==ID; });
	if(i!=mTimers.end())
	{
		auto Timer=*i;
		mTimers.erase(i);
		delete Timer;
	}
}

DWORD cTimerServer::GetTimeToNextTimer(DWORD CurrentTime) const
{
	return mTimers.empty()?INFINITE:(mTimers.front()->mNextTrigger<=CurrentTime?0:mTimers.front()->mNextTrigger-CurrentTime);
}

void cTimerServer::ProcessTimers(DWORD CurrentTime)
{
	for(;;)
	{
		if(!mTimers.empty()&&mTimers.front()->mNextTrigger<=CurrentTime)
		{
			mCurrentTimer=mTimers.pop_front();
			bool OnlyOnce=mCurrentTimer->mRequest.mFlags&cTimerRequest::OnlyOnce;
			mCurrentTimer->mListener();
			if(OnlyOnce)
			{
				delete mCurrentTimer;
				mCurrentTimer=nullptr;
				continue;
			}
			if(mCurrentTimer)
			{
				mCurrentTimer->mNextTrigger=CurrentTime+mCurrentTimer->mRequest.mInterval;
				InsertTimer(mCurrentTimer);
			}
		}
		else
			break;
	}
	mCurrentTimer=nullptr;
}

