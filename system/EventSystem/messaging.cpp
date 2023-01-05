#include "StdAfx.h"

#include "messaging.h"

void cMessageDispatcher::scheduleTopic(MessagingInternal::cMessageTopic* topic)
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