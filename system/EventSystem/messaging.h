#pragma once

namespace MessagingInternal
{

class cMessageTopic
{
public:
    virtual void dispatchMessages() = 0;
};

}

template<class MessageType> class tMessageTopic:
    public tLocatableResourceBase<tMessageTopic<MessageType>>,
    public cRegistrationHandler,
    protected cMessageTopic
{
public:
    using cListenerFunction = std::function<void(const MessageType& message)>;
    
    void post(tIntrusivePtr<MessageType> message);
    void registerListener(const cListenerFunction& listener);

private:
    std::shared_ptr<cMessageDispatcher> mDispatcher;
    virtual void dispatchMessages() override;
};

class cMessageDispatcher
{
public:
    void scheduleTopic(cMessageTopic* topic
};

template<class MessageType> 
void tMessageTopic<MessageType>::post(tIntrusivePtr<MessageType> message)
{
    mDispatcher->scheduleTopic(this);
}