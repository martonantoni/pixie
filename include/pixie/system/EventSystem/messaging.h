#pragma once


template<class C, class T> concept cMessageListener = 
    std::is_invocable_r_v<void, C, T> || std::is_invocable_r_v<void, C, const T&> || std::is_invocable_r_v<void, C>;

class cMessageCenter final
{
    static constexpr int mDirectMessageIndex = -1;
    class cDispatcher
    {
    public:
        cDispatcher() = default;
        virtual ~cDispatcher() = default;
        virtual void dispatch(const std::any& messageData, int messageIndex) = 0;
    };
    template<class T> struct tDispatcher : public cDispatcher
    {
        tDispatcher() = default;

        using cFunctions = Messaging::tMessageListeners<T>;

        struct cListener
        {
            cFunctions mFunction;
            int mEventFilter = -1;
            template<class C> cListener(const C& callable, int eventFilter);
        };
        virtual void dispatch(const std::any& messageData, int messageIndex) override;
        tRegisteredObjects<cListener> mListeners;
    };
    template<> struct tDispatcher<void> : public cDispatcher
    {
        tDispatcher() = default;
        using cFunction = std::function<void()>;
        struct cListener
        {
            cFunction mFunction;
            int mEventFilter = -1;
            cListener(const cFunction& function, int eventFilter)
                : mFunction(function), mEventFilter(eventFilter) {}
        };
        virtual void dispatch(const std::any& messageData, int messageIndex) override;
        tRegisteredObjects<cListener> mListeners;
    };
    using cVoidDispatcher = tDispatcher<void>;
    struct cUnknownMessageType {};
    struct cEndPoint
    {
        std::optional<std::type_index> mMessageType;
        std::unique_ptr<cDispatcher> mDispatcher;
        std::unique_ptr<cVoidDispatcher> mVoidDispatcher;
        void dispatch(const std::any& messageData, int messageIndex);
    };
    using cDispatchers = std::unordered_map<std::string, std::unique_ptr<cEndPoint>>;
    cDispatchers mEndPoints;
    struct cEvent
    {
        std::any mMessageData;
        cEndPoint* mDispatcher;
    };
    std::vector<cEvent> mEventsWriting;
    std::vector<cEvent> mEventsReading;
    int mLastPostedMessageIndex = -1;
    int mDispatchedMessageIndex = 0;
    std::function<void()> mNeedDispatchProcessor;
public:
    template<class... Ts> void post(const std::string& endpointID, Ts&&... messageData);
    template<class... Ts> void send(const std::string& endpointID, Ts&&... messageData);
    void post(const std::string& endpointID);
    void send(const std::string& endpointID);
    template<class C> requires cCallableSignature<C>::available [[nodiscard]] 
        cRegisteredID registerListener(const std::string& endpointID, const C& listener);
    void dispatch();
    void setNeedDispatchProcessor(std::function<void()> needDispatchProcessor);
};
 

template<class T> template<class C> 
cMessageCenter::tDispatcher<T>::cListener::cListener(const C& callable, int eventFilter)
    : mEventFilter(eventFilter)
{
    mFunction = callable;
}

template<class T>
void cMessageCenter::tDispatcher<T>::dispatch(const std::any& messageData, int messageIndex)
{
    const T& messageDataT = std::any_cast<const T&>(messageData);
    mListeners.ForEach([messageIndex, &messageDataT](auto& listener)
        {
            if (messageIndex == mDirectMessageIndex || messageIndex > listener.mEventFilter)
            {
                if(messageIndex != mDirectMessageIndex)
                    listener.mEventFilter = messageIndex;
                std::visit(
                    [&messageDataT](auto&& listener)
                    {
                        [&]<size_t... I>(std::index_sequence<I...>)
                        {
                            ([&]<size_t J>()
                            {
                                if constexpr (std::is_same_v<std::decay_t<decltype(listener)>, Messaging::tPrefixTakerFunction<J, T>>)
                                {
                                    [&] <size_t... K>(std::index_sequence<K...>)
                                    {
                                        if (!listener)
                                        {
                                            return;
                                        }
                                        listener(std::get<K>(messageDataT)...);
                                    }(std::make_index_sequence<J>());
                                }
                            }.template operator()<I>(), ...);
                        }(std::make_index_sequence<std::tuple_size_v<T> + 1>());
                    }, listener.mFunction);
            }
        });
}

//template<class T> void cMessageCenter::post(const std::string& endpointID, T&& messageData)
template<class... Ts> void cMessageCenter::post(const std::string& endpointID, Ts&&... messageData)
{
    using T = std::tuple<std::decay_t<Ts>...>;
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(T));
        endPoint->mDispatcher = std::make_unique<tDispatcher<T>>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(T))
            {
                throw std::runtime_error("Wrong message type");
            }
        }
        else
        {
            endPoint->mMessageType.emplace(typeid(T));
        }
    }
    ++mLastPostedMessageIndex;
    mEventsWriting.emplace_back(std::make_tuple(std::forward<Ts>(messageData)...), endPoint.get());
    if(mNeedDispatchProcessor)
        mNeedDispatchProcessor();
}

template<class... Ts> void cMessageCenter::send(const std::string& endpointID, Ts&&... messageData)
{
    using T = std::tuple<std::decay_t<Ts>...>;
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(T));
        endPoint->mDispatcher = std::make_unique<tDispatcher<T>>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        else
        {
            endPoint->mMessageType.emplace(typeid(T));
        }
    }
    endPoint->dispatch(std::make_tuple(std::forward<Ts>(messageData)...), mDirectMessageIndex);
}

//template<class T, class C> requires cMessageListener<C, T>
//    cRegisteredID cMessageCenter::registerListener(const std::string& endpointID, const C& listener)
//{
//}

template<class C> requires cCallableSignature<C>::available
    cRegisteredID cMessageCenter::registerListener(const std::string& endpointID, const C& listener)
{
    using T = typename cCallableSignature<C>::DecayedArguments;

    auto& endPoint = mEndPoints[endpointID];
    int messageFilter = mEventsReading.empty() ? mLastPostedMessageIndex : mDispatchedMessageIndex;
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
    }
    if constexpr (std::tuple_size_v<T> == 0)
    {
        if (!endPoint->mVoidDispatcher)
            endPoint->mVoidDispatcher = std::make_unique<tDispatcher<void>>();
        return endPoint->mVoidDispatcher->mListeners.Register(tDispatcher<void>::cListener(listener, messageFilter));
    }
    else
    {
        if (!endPoint->mMessageType.has_value())
        {
            endPoint->mMessageType = typeid(T);
            //std::cout << "registering with type: " << endPoint->mMessageType->name() << std::endl;
        }
        else
        {
            if (*endPoint->mMessageType != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        if (!endPoint->mDispatcher)
            endPoint->mDispatcher = std::make_unique<tDispatcher<T>>();
        auto dispatcherT = dynamic_cast<tDispatcher<T>*>(endPoint->mDispatcher.get());
        return dispatcherT->mListeners.Register(tDispatcher<T>::cListener(listener, messageFilter));
    }
}


extern cMessageCenter theMessageCenter;

class cMessageListeners final
{
    cRegisteredIDList mListeners;
public:
    template<class C> void listen(const std::string& endpointID, const C& callback)
    {
        mListeners.emplace_back(theMessageCenter.registerListener(endpointID, callback));
    }
    void clear()
    {
        mListeners.clear();
    }
};
