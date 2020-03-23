#pragma once

class cEventDispatcher: 
	public cRegistrationHandler,
	public tLocatableResourceBase<cEventDispatcher>   // <-- adds refcounting
{
	static cEventDispatcher mRootDispatcher;
	cRegisteredIDList mBridgeIDs;
public:
	typedef std::function<void(const cEvent &Event)> cEventListenerFunction;
	struct cEventListenerRequest
	{
		int mOrder=100;
		bool mOnlyEventsPostedAfterRegister=false; // false: even events that are not dispatched yet, but were posted earlier
	};
private:
	bool mIsCoalescing=false;
	unsigned int mLastEventIDDispatched=0;
	struct cListenerData
	{
		cEventListenerFunction mListenerFunction;
		unsigned int mEventFilter=0;

		cListenerData(cEventDispatcher *Parent, const cEventListenerFunction &Listener, const cEventListenerRequest &Request);
	private:
		tIntrusivePtr<cEventDispatcher> mParent; // just to hold a reference
	};
	tSafeObjects<cListenerData> mListeners;
	unsigned int mEventIDCounter=0;
protected:
	virtual ~cEventDispatcher()=default;
public:
	enum class eCoalescing { Coalescing, NoCoalescing };
	cEventDispatcher(eCoalescing Coalescing=eCoalescing::NoCoalescing): mIsCoalescing(Coalescing==eCoalescing::Coalescing) {}
	static tIntrusivePtr<cEventDispatcher> GetGlobalDispatcher(const cEvent::cDispatcherID &DispatcherID, eNodeCreation Creation=cEventDispatcher::CanCreate);

	void SetCoalescing() { mIsCoalescing = true; }
	void PostEvent(cEvent &&Event=cEvent());
	static void PostEvent(const cEvent::cDispatcherID &DispatcherID, cEvent &&Event=cEvent());

	cRegisteredID RegisterListener(const cEventListenerFunction &Listener, const cEventListenerRequest &Request=cEventListenerRequest());   // holds reference to EventDispatcher until Unregister
	static cRegisteredID RegisterListener(const cEvent::cDispatcherID &DispatcherID, const cEventListenerFunction &Listener, const cEventListenerRequest &Request=cEventListenerRequest());
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;

	void BridgeEventsFrom(cEventDispatcher &EventDispatcher);
//////////////////////////////////////////////////////////////////////////
	void Internal_DispatchEvent(const cEvent &Event); // only cEventCenter is allowed to call
};

class cAutoEventListener final
{
	cRegisteredID mID;
public:
	cAutoEventListener(const cEvent::cDispatcherID &DispatcherID, const cEventDispatcher::cEventListenerFunction &Listener, const cEventDispatcher::cEventListenerRequest &Request=cEventDispatcher::cEventListenerRequest())
		: mID(cEventDispatcher::GetGlobalDispatcher(DispatcherID, cEventDispatcher::CanCreate)->RegisterListener(Listener, Request)) 
	{
	}
};