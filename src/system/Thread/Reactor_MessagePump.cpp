#include "StdAfx.h"

#include "Reactor_MessagePump.h"

DWORD cReactor_MessagePump::Wait(DWORD NumberOfHandles,DWORD Timeout)
{
	if(NumberOfHandles!=mHandles.size())
	{
		ASSERT(Timeout==0);
		return cReactor::Wait(NumberOfHandles,Timeout);
	}
	DWORD WaitResult=::MsgWaitForMultipleObjectsEx(NumberOfHandles,&mHandles[0],Timeout,QS_ALLINPUT|QS_ALLEVENTS,MWMO_INPUTAVAILABLE);
	if(WaitResult-WAIT_OBJECT_0==NumberOfHandles)
	{
		MSG Message;
		while(::PeekMessage(&Message,NULL,NULL,NULL,PM_REMOVE))
		{
			::TranslateMessage(&Message);
			::DispatchMessage(&Message);
		}
		return WAIT_TIMEOUT;
	}
	return WaitResult;
}
