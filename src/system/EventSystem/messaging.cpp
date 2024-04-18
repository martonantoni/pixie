#include "StdAfx.h"

cMessageCenter theMessageCenter;

void cMessageCenter::tDispatcher<void>::dispatch(const std::any& messageData, int messageIndex)
{
    mListeners.ForEach([messageIndex](auto& listener)
        {
            if (messageIndex > listener.mEventFilter)
            {
                listener.mEventFilter = messageIndex;
                listener.mFunction();
            }
        });
}

void cMessageCenter::dispatch()
{
    for (;;) // loop until no more events to dispatch
    {
        std::swap(mEventsReading, mEventsWriting);
        if (mEventsReading.empty())
            break;
        for (auto& event : mEventsReading)
        {
            event.mDispatcher->dispatch(event.mMessageData, mDispatchedMessageIndex);
            ++mDispatchedMessageIndex;
        }
        mEventsReading.clear();
    }
}

void cMessageCenter::post(const std::string& endpointID)
{
    auto& dispatcher = mDispatchers[endpointID];
    if (!dispatcher)
        dispatcher = std::make_unique<cVoidDispatcher>();
    else
    {
        if (dispatcher->messageType() != typeid(void))
            throw std::runtime_error("Wrong message type");
    }
    ++mLastPostedMessageIndex;
    mEventsWriting.emplace_back(std::monostate(), dispatcher.get());
    if (mNeedDispatchProcessor)
        mNeedDispatchProcessor();
}

void cMessageCenter::setNeedDispatchProcessor(std::function<void()> needDispatchProcessor)
{
    mNeedDispatchProcessor = needDispatchProcessor;
}
