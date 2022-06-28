cEventDispatcher cEventDispatcher::mRootDispatcher;

tIntrusivePtr<cEventDispatcher> cEventDispatcher::GetGlobalDispatcher(const cEvent::cDispatcherID &DispatcherID, eNodeCreation Creation)
{
    ASSERT(theMainThread->IsInThread());
	return mRootDispatcher.GetSubResource(DispatcherID,Creation);
}

void cEventDispatcher::PostEvent(cEvent &&Event)
{
	if(mIsCoalescing&&mEventIDCounter>mLastEventIDDispatched)
	{
		return;
	}
// event need to be posted even if there is currently no listener, because we are supporting listeners getting the event even
//    if they subscribed after the event was posted
// the use case for this:
// obj listens for mouse left button down. if it receives that event, it starts listening for mouse left button up, it is interested in all button up events
//    which were posted after the button down event ---> the button up event might be already posted
	Event.mEventID=++mEventIDCounter;
	theEventCenter->PostEvent(std::move(Event), tIntrusivePtr<cEventDispatcher>(this));
}

/*static*/ void cEventDispatcher::PostEvent(const cEvent::cDispatcherID &DispatcherID, cEvent &&Event)
{
	GetGlobalDispatcher(DispatcherID, eNodeCreation::CanCreate)->PostEvent(std::move(Event));
}

cEventDispatcher::cListenerData::cListenerData(cEventDispatcher *Parent, const cEventListenerFunction &Listener, const cEventListenerRequest &Request)
	: mParent(Parent)
	, mEventFilter(Request.mOnlyEventsPostedAfterRegister?Parent->mEventIDCounter:0)
	, mListenerFunction(Listener)
{
}

cRegisteredID cEventDispatcher::RegisterListener(const cEvent::cDispatcherID &DispatcherID, const cEventListenerFunction &Listener, const cEventListenerRequest &Request)
{
	return GetGlobalDispatcher(DispatcherID, CanCreate)->RegisterListener(Listener, Request);
}

cRegisteredID cEventDispatcher::RegisterListener(const cEventListenerFunction &Listener, const cEventListenerRequest &Request)
{
	ASSERT(!Request.mOnlyEventsPostedAfterRegister||!mIsCoalescing);
	return cRegisteredID(this, mListeners.Register(cListenerData(this, Listener, Request)));
}

void cEventDispatcher::Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType)
{
	Ref();
	mListeners.Unregister(RegisteredID.GetID());
	Unref();
}

void cEventDispatcher::BridgeEventsFrom(cEventDispatcher &EventDispatcher)
{
	mBridgeIDs.emplace_back(EventDispatcher.RegisterListener([this](auto &Event)
	{
		Ref();
		Internal_DispatchEvent(Event);
		Unref();
	}));
}

void cEventDispatcher::Internal_DispatchEvent(const cEvent &Event)
{
	mLastEventIDDispatched=Event.mEventID;
	mListeners.ForEach([&Event](auto &Listener)
	{
		if(Event.mEventID>Listener.mEventFilter)
		{
			Listener.mEventFilter=Event.mEventID;
			Listener.mListenerFunction(Event);
		}
	});
}

