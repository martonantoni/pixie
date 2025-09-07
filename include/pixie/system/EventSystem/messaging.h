#pragma once

class cMessageIndex
{
    int mIndex;
public:
    constexpr cMessageIndex(int index = 0) : mIndex(index) {}
    constexpr explicit operator int() const { return mIndex; }
    constexpr cMessageIndex& operator++() { ++mIndex; return *this; }
    constexpr auto operator<=>(const cMessageIndex&) const = default;
};

class cMessageCenter;

class cMessageSequence
{
    cRegisteredIDList mListeners;
    cMessageIndex mOriginalMessageIndex = -1;
    cMessageCenter& mCenter;
    class cListenerWrapper
    {
        cMessageSequence& mSequence;
    protected:
        bool canDispatch(cMessageIndex messageIndex) const { return messageIndex == mSequence.mOriginalMessageIndex; }
    public:
        cListenerWrapper(cMessageSequence& sequence) : mSequence(sequence) {}
        virtual ~cListenerWrapper() = default;
    };
    template<class Listener> class tListenerWrapper;
public:
    cMessageSequence(cMessageCenter& center) : mCenter(center) {}
    cMessageSequence(const cMessageSequence&) = delete;
    cMessageSequence& operator=(const cMessageSequence&) = delete;
    cMessageSequence(cMessageSequence&&) = default;
    cMessageSequence& operator=(cMessageSequence&&) = default;
    void setFilter(cMessageIndex messageIndex) { mOriginalMessageIndex = messageIndex; }

    template<class C> requires cCallableSignature<C>::available void on(const std::string& endpointID, const C& listener);
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
    template<class... Ts> class cMessageSequenceBuilder
    {
        cMessageSequence mMessageSequence;
        std::tuple<std::decay_t<Ts>...> mMessageData;
        cMessageCenter& mCenter;
        std::string mEndPoint;
        cMessageSequenceBuilder(const cMessageSequenceBuilder&) = delete;
        cMessageSequenceBuilder(cMessageSequenceBuilder&&) = default;
        cMessageSequenceBuilder& operator=(const cMessageSequenceBuilder&) = delete;
        cMessageSequenceBuilder& operator=(cMessageSequenceBuilder&&) = delete;
    public:
        cMessageSequenceBuilder(cMessageCenter& center, const std::string& endpoint, Ts&&... messageData) :
            mMessageSequence(center),
            mCenter(center),
            mMessageData(std::forward<Ts>(messageData)...),
            mEndPoint(endpoint)
        {
        }
        auto on(const std::string& endpointID, const auto& listener) && -> cMessageSequenceBuilder<Ts...>
        {
            mMessageSequence.on(endpointID, listener);
            return std::move(*this);
        }
        operator cMessageSequence()&&
        {
            std::apply([&, this](auto&&... args)
                {
                    mMessageSequence.setFilter(mCenter.post(mEndPoint, std::forward<decltype(args)>(args)...));
                },
                std::move(mMessageData));
            return std::move(mMessageSequence);
        }
    };
public:
    template<class... Ts> cMessageIndex post(const std::string& endpointID, Ts&&... messageData);
    cMessageIndex post(const std::string& endpointID);
    template<class... Ts> void send(const std::string& endpointID, Ts&&... messageData);
    void send(const std::string& endpointID);

    template<class... Ts> auto sequence(const std::string& endpointID, Ts&&... messageData);
    template<class C> requires cCallableSignature<C>::available [[nodiscard]] 
        cRegisteredID registerListener(const std::string& endpointID, const C& listener);
    void dispatch();
    void setNeedDispatchProcessor(std::function<void()> needDispatchProcessor);
};

// MessageSequence

template<class R, class... Args> class cMessageSequence::tListenerWrapper<R(Args...)> : public cMessageSequence::cListenerWrapper
{
    std::function<void(Args...)> mListener;
public:
    tListenerWrapper(cMessageSequence& sequence, const std::function<void(Args...)>& listener)
        : cListenerWrapper(sequence), mListener(listener) {
    }
    void operator()(cMessageIndex messageIndex, Args... args)
    {
        if (canDispatch(messageIndex))
            mListener(std::forward<Args>(args)...);
    }
};

template<class C> requires cCallableSignature<C>::available
void cMessageSequence::on(const std::string& endpointID, const C& listener)
{
    mListeners.emplace_back(mCenter.registerListener(endpointID,
        tListenerWrapper<typename cSignatureExtractor<decltype(&C::operator())>::Signature>(*this, listener)));
}

// MessageCenter

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

    mListeners.ForEach([messageIndex, messageDataT, messageDataIndexedT](auto& listener)
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
                    else
                        applyTail<std::tuple_size_v<T>>(std::get<1>(listener.mFunction), *messageDataIndexedT);
                    break;
                case 2:
                    if (messageDataT)
                        std::apply(std::get<2>(listener.mFunction),
                            std::tuple_cat(std::forward_as_tuple(messageIndex), *messageDataT));
                    else
                        std::apply(std::get<2>(listener.mFunction), *messageDataIndexedT);
                    break;
                };
            }
        });
}

template<class... Ts> 
cMessageIndex cMessageCenter::post(const std::string& endpointID, Ts&&... messageData)
{
    using AllT = std::tuple<std::decay_t<Ts>...>;
    using T = std::conditional_t<
        std::tuple_size_v<AllT> >= 1 &&        
        std::is_same_v<tSafeTupleElementT<0, AllT>, cMessageIndex>,
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
                throw std::runtime_error(std::format("Wrong message type (post), expected: {}, got: {}", 
                    endPoint->mMessageType->name(), typeid(T).name()));
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
    return mLastPostedMessageIndex;
}

template<class... Ts> auto cMessageCenter::sequence(const std::string& endpoint, Ts&&... messageData)
{
    return cMessageSequenceBuilder<Ts...>{ *this, endpoint, std::forward<Ts>(messageData)... };
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
                throw std::runtime_error("Wrong message type (send)");
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
    int numberOfArgs = Signature::numberOfArguments;
    using T = std::conditional_t<
        Signature::numberOfArguments >= 1 &&         
        std::is_same_v<tSafeTupleElementT<0, typename Signature::DecayedArguments>, cMessageIndex>,
        tTuplePostfix<typename Signature::DecayedArguments, Signature::numberOfArguments - 1>,
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
                throw std::runtime_error("Wrong message type (listener)");
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
