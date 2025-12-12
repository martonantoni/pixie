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
    try
    {
        theEventCenter->DispatchEvents(mTimeToNextTimer);
        theMessageCenter.dispatch();
    }
    catch(std::runtime_error& e)
    {
        MainLog->Log("Exception in main thread event dispatch: %s", e.what());
    }
}


cMainThread* theMainThread = NULL;