#pragma once

struct cEvent
{
	typedef cResourceLocation cDispatcherID;
	
	cRegisteredID mEventDataID;

	cEvent()=default;
	cEvent(cRegisteredID &&EventDataID): mEventDataID(std::move(EventDataID)) {}
	cEvent(const cEvent &Other)=delete;
	cEvent(cEvent &&Other)=default;
	cEvent &operator=(const cEvent &Other)=delete;
	cEvent &operator=(cEvent &&Other)=default;
	virtual ~cEvent()=default;
private:
	friend class cEventDispatcher;
	unsigned int mEventID=0;
};

