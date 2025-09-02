#include "StdAfx.h"

cMessageCenter theMessageCenter;

void cMessageCenter::tDispatcher<void>::dispatch(const std::any& messageData, cMessageIndex messageIndex)
{
    mListeners.ForEach([messageIndex](auto& listener)
        {
            if (messageIndex == mDirectMessageIndex || messageIndex > listener.mEventFilter)
            {
                listener.mEventFilter = messageIndex;
                std::get<1>(listener.mFunction)();
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
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(void));
        endPoint->mVoidDispatcher = std::make_unique<cVoidDispatcher>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(void))
                throw std::runtime_error("Wrong message type");
        }
        else
            endPoint->mMessageType.emplace(typeid(void));
    }
    ++mLastPostedMessageIndex;
    mEventsWriting.emplace_back(std::monostate(), endPoint.get());
    if (mNeedDispatchProcessor)
        mNeedDispatchProcessor();
}

void cMessageCenter::send(const std::string& endpointID)
{
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(void));
        endPoint->mVoidDispatcher = std::make_unique<cVoidDispatcher>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(void))
                throw std::runtime_error("Wrong message type");
        }
        else
            endPoint->mMessageType.emplace(typeid(void));
    }
    endPoint->dispatch(std::monostate(), mDirectMessageIndex);
}
 
void cMessageCenter::setNeedDispatchProcessor(std::function<void()> needDispatchProcessor)
{
    mNeedDispatchProcessor = needDispatchProcessor;
}

void cMessageCenter::cEndPoint::dispatch(const std::any& messageData, cMessageIndex messageIndex)
{
    if (mDispatcher)
        mDispatcher->dispatch(messageData, messageIndex);
    if (mVoidDispatcher)
        mVoidDispatcher->dispatch(messageData, messageIndex);
}