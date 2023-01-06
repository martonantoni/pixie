#pragma once

class cMessageDispatcher;

class cMessageBase
{
public:
    virtual ~cMessageBase() = default;
};

class cMessageTopic:
    public tLocatableResourceBase<cMessageTopic>,
    public cRegistrationHandler
{
public:
    using cMessagePtr = std::unique_ptr<const cMessageBase>;
    using cListenerFunction = std::function<void(const cMessageBase&)>;

    void post(cMessagePtr message);
    cRegisteredID registerListener(const cListenerFunction& listeningFunction);

    virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType = eCallbackType::Wait) override;
    void dispatchMessages();
private:
    std::shared_ptr<cMessageDispatcher> mDispatcher;
    bool mIsScheduled = false;
    tSafeObjects<cListenerFunction> mListeners;
    std::vector<cMessagePtr> mMessagesWriting, mMessagesReading;
};

class cMessageDispatcher
{
public:
    using cActivationCallback = std::function<void(cMessageDispatcher&)>;
    void scheduleTopic(cMessageTopic* topic);
    void dispatchAll();
    void setActivationCallback(const cActivationCallback& callback);
private:
    cActivationCallback mActivationCallback;
    std::vector<cMessageTopic*> mScheduledTopicsWriting, mScheduledTopicsReading;
};


