#pragma once

struct cTimerRequest
{
	enum eFlags
	{
		UseFrameLockedTime = 1, // GetTickCount is locked to the frame's start time.
		OnlyOnce           = 2 // timer will be called only once. ID is still returned! (you can use theIDSink if you don't need it)
	};
	DWORD mInterval;
	int mFlags = 0;
	cTimerRequest(DWORD Interval,int Flags=0): mInterval(Interval), mFlags(Flags) {}
	cTimerRequest(): mFlags(0) {}
	static cTimerRequest once(DWORD Interval) { return cTimerRequest(Interval,OnlyOnce); }
};