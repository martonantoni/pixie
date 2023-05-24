#pragma once

class cMutex
{
	mutable CRITICAL_SECTION CriticalSection;
public:
	inline cMutex()  { InitializeCriticalSectionAndSpinCount(&CriticalSection,4000); }
	inline ~cMutex() { DeleteCriticalSection(&CriticalSection);  }
	inline void Lock() const    { EnterCriticalSection(&CriticalSection); }
	inline void Release() const { LeaveCriticalSection(&CriticalSection);  }
	inline void Acquire() const { Lock(); }
};

class cMutexGuard
{
	const cMutex &Mutex;
	int LockCounter;
public:
	inline cMutexGuard(const cMutex &pMutex,int NeedLock=true): Mutex(pMutex) { LockCounter=0; if(NeedLock) Lock(); }
	inline void Lock()    { Mutex.Lock(); ++LockCounter; }
	inline void Release() { if(LockCounter) { Mutex.Release(); --LockCounter; } }
	inline ~cMutexGuard() { while(LockCounter) Release(); }
};
