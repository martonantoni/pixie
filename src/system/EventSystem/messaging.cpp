#include "StdAfx.h"


cRegisteredID cMessageTopic::registerListener(const cListenerFunction& listeningFunction)
{
    return cRegisteredID(this, mListeners.Register(listeningFunction));
}

void cMessageTopic::dispatchMessages()
{
    mIsScheduled = false;
    std::swap(mMessagesReading, mMessagesWriting);
    for (auto& message : mMessagesReading)
    {
        mListeners.ForEach([&message](auto& listener)
            {
                listener(*message);
            });
    }
    mMessagesReading.clear();
}

void cMessageTopic::Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType)
{
    mListeners.Unregister(RegisteredID.GetID());
}

void cMessageTopic::post(cMessagePtr message)
{
    mMessagesWriting.emplace_back(std::move(message));
    if (!mIsScheduled)
    {
        mDispatcher->scheduleTopic(this);
    }
}

//////////////////////////////////////////////////////////////////////////

void cMessageDispatcher::scheduleTopic(cMessageTopic* topic)
{
    mScheduledTopicsWriting.emplace_back(topic);
    if (mScheduledTopicsWriting.size() == 1 && mActivationCallback)
    {
        mActivationCallback(*this);
    }
}

void cMessageDispatcher::dispatchAll()
{
    std::swap(mScheduledTopicsWriting, mScheduledTopicsReading);
    for (auto& topic : mScheduledTopicsReading)
    {
        topic->dispatchMessages();
    }
    mScheduledTopicsReading.clear();
}