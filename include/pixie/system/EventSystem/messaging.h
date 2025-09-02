#pragma once

class cMessageIndex // todo: create strong type support class
{
    int mIndex;
public:
    constexpr cMessageIndex(int index = 0) : mIndex(index) {}
    constexpr explicit operator int() const { return mIndex; }
    constexpr cMessageIndex& operator++() { ++mIndex; return *this; }
    constexpr auto operator<=>(const cMessageIndex&) const = default;
};

class cMessageSequence
{
    cRegisteredIDList mListeners;
public:
    cMessageSequence() = default;
    cMessageSequence(const cMessageSequence&) = delete;
    cMessageSequence& operator=(const cMessageSequence&) = delete;
    cMessageSequence(cMessageSequence&&);
    cMessageSequence& operator=(cMessageSequence&&);

    template<class C> requires cCallableSignature<C>::available [[nodiscard]]
        cMessageSequence response(const std::string& endpointID, const C& listener);
};

template <typename C, typename TTuple>
struct is_invocable_with_tuple;

template <typename C, typename... Args>
struct is_invocable_with_tuple<C, std::tuple<Args...>>
    : std::is_invocable<C, Args...> {
};


class cMessageCenter final
{
    static constexpr cMessageIndex mDirectMessageIndex = -1;
    class cDispatcher
    {
    public:
        cDispatcher() = default;
        virtual ~cDispatcher() = default;
        virtual void dispatch(const std::any& messageData, cMessageIndex messageIndex) = 0;
    };
    template<class T> struct tDispatcher : public cDispatcher
    {
        tDispatcher() = default;

        using cFunction = tTupleTakerFunction<T>;
        using cMessageIndexedT = tTuplePrepend<T, cMessageIndex>;
        using cMessageIndexTakerFunction = tTupleTakerFunction<cMessageIndexedT>;

        struct cListener
        {
            std::variant<std::monostate, cFunction, cMessageIndexTakerFunction> mFunction;
            cMessageIndex mEventFilter = -1;
            template<class C> cListener(const C& callable, cMessageIndex eventFilter);
        };
        virtual void dispatch(const std::any& messageData, cMessageIndex messageIndex) override;
        tRegisteredObjects<cListener> mListeners;
    };
    template<> struct tDispatcher<void> : public cDispatcher             // void dispatcher
    {
        tDispatcher() = default;
        using cFunction = std::function<void()>;
        using cMessageIndexTakerFunction = std::function<void(cMessageIndex)>;
        struct cListener
        {
            std::variant<std::monostate, cFunction, cMessageIndexTakerFunction> mFunction;
            cMessageIndex mEventFilter = -1;
            template<class C> cListener(const C& callable, cMessageIndex eventFilter);
        };
        virtual void dispatch(const std::any& messageData, cMessageIndex messageIndex) override;
        tRegisteredObjects<cListener> mListeners;
    };
    using cVoidDispatcher = tDispatcher<void>;
    struct cUnknownMessageType {};
    struct cEndPoint
    {
        std::optional<std::type_index> mMessageType;
        std::unique_ptr<cDispatcher> mDispatcher;
        std::unique_ptr<cVoidDispatcher> mVoidDispatcher;
        void dispatch(const std::any& messageData, cMessageIndex messageIndex);
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
    cMessageIndex mLastPostedMessageIndex = -1;
    cMessageIndex mDispatchedMessageIndex = 0;
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
cMessageCenter::tDispatcher<T>::cListener::cListener(const C& callable, cMessageIndex eventFilter)
    : mEventFilter(eventFilter)
{
    if constexpr (is_invocable_with_tuple<C, T>::value)
        mFunction = cFunction(callable);
    else 
        mFunction = cMessageIndexTakerFunction(callable);
}

template<class C>
inline cMessageCenter::tDispatcher<void>::cListener::cListener(const C& callable, cMessageIndex eventFilter)
    : mEventFilter(eventFilter)
{
    if constexpr (std::is_invocable_v<C>)
        mFunction = cFunction(callable);
    else
        mFunction = cMessageIndexTakerFunction(callable);
}

template<class T>
void cMessageCenter::tDispatcher<T>::dispatch(const std::any& messageData, cMessageIndex messageIndex)
{
    const T* messageDataT = std::any_cast<const T>(&messageData);
    const cMessageIndexedT* messageDataIndexedT = std::any_cast<const cMessageIndexedT>(&messageData);

    //if (!messageDataT)
    //    messageDataT = 
    mListeners.ForEach([messageIndex, messageDataT](auto& listener)
        {
            if (messageIndex == mDirectMessageIndex || messageIndex > listener.mEventFilter)
            {
                if(messageIndex != mDirectMessageIndex)
                    listener.mEventFilter = messageIndex;
                switch (listener.mFunction.index())
                {
                case 1:
                    if(messageDataT)
                        std::apply(std::get<1>(listener.mFunction), *messageDataT);
                    //else
                    //    std::apply(std::get<1>(listener.mFunction), );
                    break;
                case 2:
                    std::apply(std::get<2>(listener.mFunction), 
                        std::tuple_cat(std::forward_as_tuple(messageIndex), *messageDataT));
                    break;
                };
            }
        });
}

template<class... Ts> void cMessageCenter::post(const std::string& endpointID, Ts&&... messageData)
{
    using AllT = std::tuple<std::decay_t<Ts>...>;
    using T = std::conditional_t<
        std::tuple_size_v<AllT> >= 1 &&        
        std::is_same_v<tSafeTupleElement<0, AllT>, cMessageIndex>,
        tTuplePostfix<AllT, std::tuple_size_v<AllT> -1>,
        AllT>;
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
    using AllT = std::tuple<std::decay_t<Ts>...>;
    using T = std::conditional_t<
        std::tuple_size_v<AllT> >= 1 &&
        std::is_same_v<tSafeTupleElement<0, AllT>, cMessageIndex>,
        tTuplePostfix<AllT, std::tuple_size_v<AllT> -1>,
        AllT>;
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

template<class C> requires cCallableSignature<C>::available
    cRegisteredID cMessageCenter::registerListener(const std::string& endpointID, const C& listener)
{
    using Signature = cCallableSignature<C>;
    using T = std::conditional_t<
        Signature::numberOfArguments >= 1 &&         
        std::is_same_v<tSafeTupleElement<0, typename Signature::DecayedArguments>, cMessageIndex>,
        typename Signature::DecayedArguments,
        typename Signature::DecayedArguments>;

    auto& endPoint = mEndPoints[endpointID];
    auto messageFilter = mEventsReading.empty() ? mLastPostedMessageIndex : mDispatchedMessageIndex;
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
