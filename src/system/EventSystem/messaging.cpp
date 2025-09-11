#include "StdAfx.h"

namespace 
{ 
    // hack for backward compatibility
std::shared_ptr<cMessageCenter> theMessageCenterPtr = std::make_shared<cMessageCenter>();
}
cMessageCenter& theMessageCenter = *theMessageCenterPtr;

cMessageSequence::cMessageSequence(cMessageSequence&& src):
    mCenter(src.mCenter),
    mListeners(std::move(src.mListeners)),
    mFilter(src.mFilter)
{
}

cMessageSequence& cMessageSequence::operator=(cMessageSequence&& src)
{
    if (this != &src)
    {
        // use move assignment to avoid unregistering listeners
        mCenter = src.mCenter;
        mListeners = std::move(src.mListeners);
        mFilter = src.mFilter;
    }
    return *this;
}


bool cMessageSequence::cListenerWrapper::canDispatch(cMessageSequencingID sequencingID) const
{ 
    return sequencingID.mInResponseTo == mFilter;
}

void cMessageCenter::tDispatcher<void>::dispatch(const std::any& messageData, cMessageSequencingID sequencingID)
{
    mListeners.ForEach([sequencingID](auto& listener)
        {
            if (sequencingID.mThisMessage == mDirectMessageIndex || sequencingID.mThisMessage > listener.mEventFilter)
            {
                listener.mEventFilter = sequencingID.mThisMessage;
                switch (listener.mFunction.index())
                {
                case 1:
                    std::get<1>(listener.mFunction)();
                    break;
                case 2:
                    std::get<2>(listener.mFunction)(sequencingID);
                    break;
                }
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
            event.mDispatcher->dispatch(event.mMessageData, 
                cMessageSequencingID
                { 
                    .mInResponseTo = event.mInResponseTo,
                    .mThisMessage = mDispatchedMessageIndex
                });
            ++mDispatchedMessageIndex;
        }
        mEventsReading.clear();
    }
}

cMessageIndex cMessageCenter::post(const std::string& endpointID)
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
                throw std::runtime_error("Wrong message type (post)");
        }
        else
            endPoint->mMessageType.emplace(typeid(void));
    }
    ++mLastPostedMessageIndex;
    mEventsWriting.emplace_back(std::monostate(), endPoint.get());
    if (mNeedDispatchProcessor)
        mNeedDispatchProcessor();
    return mLastPostedMessageIndex;
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
                throw std::runtime_error("Wrong message type (send)");
        }
        else
            endPoint->mMessageType.emplace(typeid(void));
    }
    endPoint->dispatch(std::monostate(), 
        cMessageSequencingID
        {
            .mInResponseTo = cMessageIndex::invalid(),
            .mThisMessage = mDirectMessageIndex
        });
}
 
void cMessageCenter::setNeedDispatchProcessor(std::function<void()> needDispatchProcessor)
{
    mNeedDispatchProcessor = needDispatchProcessor;
}

void cMessageCenter::cEndPoint::dispatch(const std::any& messageData, cMessageSequencingID sequencingID)
{
    if (mDispatcher)
        mDispatcher->dispatch(messageData, sequencingID);
    if (mVoidDispatcher)
        mVoidDispatcher->dispatch(messageData, sequencingID);
}