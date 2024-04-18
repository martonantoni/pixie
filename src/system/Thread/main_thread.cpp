#include "StdAfx.h"

#include "pixie/System/EventSystem/i_EventSystem.h"

void cMainThread::threadLoop()
{
    mGotEventToDispatchID = AddEventHandler([this]() { processEventDispatch(); }, &mGotEventToDispatch);
    theEventCenter->setNeedDispatchProcessor([this]() { mGotEventToDispatch.Set(); });
    theMessageCenter.setNeedDispatchProcessor([this]() { mGotEventToDispatch.Set(); });
    DWORD CurrentTime = GetTickCount();
    for (;;)
    {
        mTimeToNextTimer = mTimerServer.GetTimeToNextTimer(CurrentTime);
        mReactor->DispatchEvents(mTimeToNextTimer);
        CurrentTime = GetTickCount();
        mTimerServer.ProcessTimers(CurrentTime);
    }
}

void cMainThread::processEventDispatch()
{
    theEventCenter->DispatchEvents(mTimeToNextTimer);
    theMessageCenter.dispatch();
}


cMainThread* theMainThread = NULL;