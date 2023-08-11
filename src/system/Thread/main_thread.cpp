#include "StdAfx.h"


void cMainThread::threadLoop()
{
    //	mProcessEventDispatchID = AddEventHandler([this]() { processEventDispatch(); }, &mEventDispatchRequestedEvent);
    DWORD CurrentTime = GetTickCount();
    for (;;)
    {
        DWORD TimeToNextTimer = mTimerServer.GetTimeToNextTimer(CurrentTime);
        mReactor->DispatchEvents(TimeToNextTimer);
        CurrentTime = GetTickCount();
        mTimerServer.ProcessTimers(CurrentTime);
    }

}

cMainThread* theMainThread = NULL;