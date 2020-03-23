#pragma once

struct cTimerRequest
{
	enum eFlags
	{
		UseFrameLockedTime = 1, // GetTickCount is locked to the frame's start time.
		OnlyOnce           = 2 // timer will be called only once. ID is still returned! (you can use theIDSink if you don't need it)
	};
	DWORD mInterval;
	int mFlags;
	cTimerRequest(DWORD Interval,int Flags=0): mInterval(Interval), mFlags(Flags) {}
	cTimerRequest(): mFlags(0) {}
};