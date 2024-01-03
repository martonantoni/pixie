#pragma once

class cReactor
{
public:
	typedef std::function<void()> cEventListener;
protected:
	typedef std::vector<cEventListener> cEventListeners;
	typedef std::vector<HANDLE> cHandles;
	typedef std::vector<size_t> cIDs;
	cEventListeners mEventListeners;
	cHandles mHandles;
	cIDs mIDs;
	size_t mIDCounter=0;
	bool mAddOrRemoveTrap=false;
	void SwapListeners(size_t First, size_t Second);
	virtual DWORD Wait(DWORD NumberOfHandles, DWORD Timeout);
public:
	cReactor();
	virtual ~cReactor();
	size_t AddEventListener(const cEventListener &EventListener,cNativeEvent *Event);
	void RemoveEventListener(size_t ID);
	bool DispatchEvents(DWORD Timeout); //true: at least one event was dispatched
};