#pragma once

namespace MessagingInternal
{

class cMessageTopic
{
public:
    virtual void dispatchMessages() = 0;
};

}

class cMessageDispatcher;

template<class MessageType> class tMessageTopic:
    public tLocatableResourceBase<tMessageTopic<MessageType>>,
    public cRegistrationHandler,
    protected MessagingInternal::cMessageTopic
{
public:
    using cListenerFunction = std::function<void(const MessageType& message)>;
    
    void post(tIntrusivePtr<MessageType> message);
    cRegisteredID registerListener(const cListenerFunction& listener);
    virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType = eCallbackType::Wait) override;

private:
    std::shared_ptr<cMessageDispatcher> mDispatcher;
    bool mIsScheduled = false;
    tSafeObjects<cListenerFunction> mListeners;
    std::vector<tIntrusivePtr<MessageType>> mMessagesWriting, mMessagesReading;
    virtual void dispatchMessages() override;
};

class cMessageDispatcher
{
public:
    using cActivationCallback = std::function<void(cMessageDispatcher&)>;
    void scheduleTopic(MessagingInternal::cMessageTopic* topic);
    void dispatchAll();
    void setActivationCallback(const cActivationCallback& callback);
private:
    cActivationCallback mActivationCallback;
    std::vector<MessagingInternal::cMessageTopic*> mScheduledTopicsWriting, mScheduledTopicsReading;
};


template<class MessageType>
cRegisteredID tMessageTopic<MessageType>::registerListener(const cListenerFunction& listener)
{
    return cRegisteredID(this, mListeners.Register(cListenerFunction));
}

template<class MessageType>
void tMessageTopic<MessageType>::dispatchMessages()
{
    mIsScheduled = false;
    std::swap(mMessagesReading, mMessageWriting);
    for (auto& message : mMessagesReading)
    {
        mListeners.ForEach([&message](auto& listener)
        {
            listener(*message);
        });
    }
    mMessagesReading.clear();
}

template<class MessageType>
void tMessageTopic<MessageType>::Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType)
{
    mListeners.Unregister(RegisteredID.GetID());
}


template<class MessageType> 
void tMessageTopic<MessageType>::post(tIntrusivePtr<MessageType> message)
{
    mMessagesWriting.emplace_back(std::move(message));
    if (!mIsScheduled)
    {
        mDispatcher->scheduleTopic(this);
    }
}