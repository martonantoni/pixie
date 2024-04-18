#pragma once


class cMessageCenter
{
    struct cDispatcher : public cRegistrationHandler
    {
        std::type_index mTypeIndex;
        virtual ~cDispatcher() = default;
        cDispatcher(const std::type_index& typeIndex) : mTypeIndex(typeIndex) {}
        virtual void dispatch(const std::any& messageData, int messageIndex) = 0;
    };
    template<class T> struct tDispatcher : public cDispatcher
    {
        using cFunctions = std::variant<std::monostate, std::function<void(const T&)>, std::function<void(T)>, std::function<void()>>;
        struct cListener
        {
            cFunctions mFunction;
            int mEventFilter = -1;
            template<class C> cListener(const C& callable, int eventFilter)
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
        };
        using cListeners = tSafeObjects<cListener>;
        cListeners mListeners;
        virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType) override
        {
            mListeners.Unregister(RegisteredID.GetID());
        }
        tDispatcher() : cDispatcher(typeid(T)) {}
        virtual void dispatch(const std::any& messageData, int messageIndex) override
        {
            const T& messageDataT = std::any_cast<const T&>(messageData);
            mListeners.ForEach([messageIndex, &messageDataT](auto& listener)
                {
                    if (messageIndex > listener.mEventFilter)
                    {
                        listener.mEventFilter = messageIndex;
                        std::visit(
                            [&messageDataT](auto&& f)
                            {
                                if constexpr (std::is_same_v<std::decay_t<decltype(f)>, std::function<void(const T&)>>)
                                    f(messageDataT);
                                else if constexpr (std::is_same_v<std::decay_t<decltype(f)>, std::function<void(T)>>)
                                    f(messageDataT);
                                else if constexpr (std::is_same_v<std::decay_t<decltype(f)>, std::function<void()>>)
                                    f();
                            },
                            listener.mFunction);
                    }
                });
        }
    };
    template<> struct tDispatcher<void> : public cDispatcher
    {
        using cFunction = std::function<void()>;
        struct cListener
        {
            cFunction mFunction;
            int mEventFilter = -1;
            cListener(const cFunction& function, int eventFilter)
                : mFunction(function), mEventFilter(eventFilter) {}
        };
        using cListeners = tSafeObjects<cListener>;
        cListeners mListeners;
        tDispatcher() : cDispatcher(typeid(void)) {}
        virtual void dispatch(const std::any& messageData, int messageIndex) override
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
        virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType) override
        {
            mListeners.Unregister(RegisteredID.GetID());
        }
    };
    using cVoidDispatcher = tDispatcher<void>;
    std::unordered_map<std::string, std::unique_ptr<cDispatcher>> mDispatchers;
    struct cEvent
    {
        std::any mMessageData;
        cDispatcher* mDispatcher;
    };
    std::vector<cEvent> mEventsWriting;
    std::vector<cEvent> mEventsReading;
    int mLastPostedMessageIndex = -1;
    int mDispatchedMessageIndex = 0;
public:
    template<class T> void post(const std::string& endPointID, T&& messageData)
    {
        auto& dispatcher = mDispatchers[endPointID];
        if (!dispatcher)
            dispatcher = std::make_unique<tDispatcher<T>>();
        else
        {
            if (dispatcher->mTypeIndex != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        ++mLastPostedMessageIndex;
        mEventsWriting.emplace_back(std::forward<T>(messageData), dispatcher.get());
    }
    void post(const std::string& endPointID)
    {
        auto& dispatcher = mDispatchers[endPointID];
        if (!dispatcher)
            dispatcher = std::make_unique<cVoidDispatcher>();
        else
        {
            if (dispatcher->mTypeIndex != typeid(void))
                throw std::runtime_error("Wrong message type");
        }
        ++mLastPostedMessageIndex;
        mEventsWriting.emplace_back(std::monostate(), dispatcher.get());
    }
    template<class T, class C> [[nodiscard]] cRegisteredID registerListener(const std::string& endPointID, const C& listener)
    {
        auto& dispatcher = mDispatchers[endPointID];
        if (!dispatcher)
            dispatcher = std::make_unique<tDispatcher<T>>();
        auto dispatcherT = dynamic_cast<tDispatcher<T>*>(dispatcher.get());
        if (!dispatcherT)
            throw std::runtime_error("Wrong message type");
        return cRegisteredID(dispatcher.get(),
            dispatcherT->mListeners.Register(tDispatcher<T>::cListener(listener, mLastPostedMessageIndex)));
    }
    void dispatch()
    {
        std::swap(mEventsReading, mEventsWriting);
        for (auto& event : mEventsReading)
        {
            event.mDispatcher->dispatch(event.mMessageData, mDispatchedMessageIndex);
            ++mDispatchedMessageIndex;
        }
    }
};
