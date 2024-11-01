#pragma once

template<class C, class T> concept cMessageListener = 
    std::is_invocable_r_v<void, C, T> || std::is_invocable_r_v<void, C, const T&> || std::is_invocable_r_v<void, C>;

//template<

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
        using cFunctions = std::variant<std::monostate, std::function<void(const T&)>, std::function<void(T)>, std::function<void()>>;
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
    template<class T> void send(const std::string& endpointID, T&& messageData);
    void post(const std::string& endpointID);
    void send(const std::string& endpointID);
    template<class T = void, class C> requires cMessageListener<C, T>
        [[nodiscard]] cRegisteredID registerListener(const std::string& endpointID, const C& listener);
    template<class C> requires cCallableSignature<C>::available [[nodiscard]] 
        cRegisteredID registerListener2(const std::string& endpointID, const C& listener);
    void dispatch();
    void setNeedDispatchProcessor(std::function<void()> needDispatchProcessor);
};
 

template<class T> template<class C> 
cMessageCenter::tDispatcher<T>::cListener::cListener(const C& callable, int eventFilter)
    : mEventFilter(eventFilter)
{
    if constexpr (std::is_same_v<C, std::function<void(const T&)>>)
        mFunction = callable;
    else if constexpr (std::is_same_v<C, std::function<void(T)>>)
        mFunction = callable;
    else if constexpr (std::is_same_v<C, std::function<void()>>)
        mFunction = callable;
    else if constexpr (std::is_invocable_r_v<void, C, T>)
        mFunction = std::function<void(T)>(callable);
    else if constexpr (std::is_invocable_r_v<void, C, const T&>)
        mFunction = std::function<void(const T&)>(callable);
    else if constexpr (std::is_invocable_r_v<void, C>)
        mFunction = std::function<void()>(callable);
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
                    [&messageDataT](auto& function)
                    {
                        if constexpr (std::is_same_v<std::decay_t<decltype(function)>, std::function<void(const T&)>>)
                            function(messageDataT);
                        else if constexpr (std::is_same_v<std::decay_t<decltype(function)>, std::function<void(T)>> && std::is_copy_constructible_v<T>)
                            function(messageDataT);
                        else if constexpr (std::is_same_v<std::decay_t<decltype(function)>, std::function<void()>>)
                            function();
                    },
                    listener.mFunction);
            }
        });
}

//template<class T> void cMessageCenter::post(const std::string& endpointID, T&& messageData)
template<class... Ts> void cMessageCenter::post(const std::string& endpointID, Ts&&... messageData)
{
    using T = std::tuple<Ts...>;
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(std::decay_t<T>));
        endPoint->mDispatcher = std::make_unique<tDispatcher<std::decay_t<T>>>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(std::decay_t<T>))
                throw std::runtime_error("Wrong message type");
        }
        else
        {
            endPoint->mMessageType.emplace(typeid(std::decay_t<T>));
        }
    }
    ++mLastPostedMessageIndex;
    mEventsWriting.emplace_back(T(std::forward<Ts>(messageData)...), endPoint.get());
    if(mNeedDispatchProcessor)
        mNeedDispatchProcessor();
}

template<class T> void cMessageCenter::send(const std::string& endpointID, T&& messageData)
{
    auto& endPoint = mEndPoints[endpointID];
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
        endPoint->mMessageType.emplace(typeid(std::decay_t<T>));
        endPoint->mDispatcher = std::make_unique<tDispatcher<std::decay_t<T>>>();
    }
    else
    {
        // for posting the message type has to match even if it is void
        if (endPoint->mMessageType.has_value())
        {
            if (*endPoint->mMessageType != typeid(std::decay_t<T>))
                throw std::runtime_error("Wrong message type");
        }
        else
        {
            endPoint->mMessageType.emplace(typeid(std::decay_t<T>));
        }
    }
    endPoint->dispatch(std::forward<T>(messageData), mDirectMessageIndex);
}

template<class T, class C> requires cMessageListener<C, T>
    cRegisteredID cMessageCenter::registerListener(const std::string& endpointID, const C& listener)
{
    auto& endPoint = mEndPoints[endpointID];
    int messageFilter = mEventsReading.empty() ? mLastPostedMessageIndex : mDispatchedMessageIndex;
    if (!endPoint)
    {
        endPoint = std::make_unique<cEndPoint>();
    }
    if constexpr (std::is_same_v<T, void>)
    {
        if (!endPoint->mVoidDispatcher)
            endPoint->mVoidDispatcher = std::make_unique<tDispatcher<void>>();
        return endPoint->mVoidDispatcher->mListeners.Register(tDispatcher<void>::cListener(listener, messageFilter));
    }
    else
    {
        if (!endPoint->mMessageType.has_value())
        {
            endPoint->mMessageType = typeid(std::decay_t<T>);
        }
        else
        {
            if (*endPoint->mMessageType != typeid(std::decay_t<T>))
                throw std::runtime_error("Wrong message type");
        }
        if (!endPoint->mDispatcher)
            endPoint->mDispatcher = std::make_unique<tDispatcher<std::decay_t<T>>>();
        auto dispatcherT = dynamic_cast<tDispatcher<std::decay_t<T>>*>(endPoint->mDispatcher.get());
        return dispatcherT->mListeners.Register(tDispatcher<std::decay_t<T>>::cListener(listener, messageFilter));
    }
}

template<class C> requires cCallableSignature<C>::available
    cRegisteredID cMessageCenter::registerListener2(const std::string& endpointID, const C& listener)
{
    using T = typename std::tuple_element_t<0, typename cCallableSignature<C>::DecayedArguments>;
    return registerListener<T>(endpointID, listener);
}


extern cMessageCenter theMessageCenter;

class cMessageListeners final
{
    cRegisteredIDList mListeners;
public:
    template<class T, class C> void listen(const std::string& endpointID, C&& callback)
    {
        mListeners.emplace_back(theMessageCenter.registerListener<T>(endpointID, std::forward<C>(callback)));
    }
    template<class C> void listen(const std::string& endpointID, C&& callback)
    {
        mListeners.emplace_back(theMessageCenter.registerListener<void>(endpointID, std::forward<C>(callback)));
    }
    void clear()
    {
        mListeners.clear();
    }
};
