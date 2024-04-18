#include "event_tests_common.h"

class cMessageCenter
{
    struct cDispatcher: public cRegistrationHandler
    {
        std::type_index mTypeIndex;
        cDispatcher(const std::type_index& typeIndex): mTypeIndex(typeIndex) {}
        virtual void dispatch(const std::any& messageData, int messageIndex) = 0;
    };
    template<class T> struct tDispatcher: public cDispatcher
    {
        struct cListener
        {
            std::function<void(const T&)> mFunction;
            int mEventFilter = -1;
        };
        using cListeners = tSafeObjects<cListener>;
        cListeners mListeners;
        virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType) override
        {
            mListeners.Unregister(RegisteredID.GetID());
        }
        tDispatcher(): cDispatcher(typeid(T)) {}
        virtual void dispatch(const std::any& messageData, int messageIndex) override
        {
            const T& messageDataT = std::any_cast<const T&>(messageData);
            mListeners.ForEach([messageIndex, &messageDataT](auto& listener)
                {
                    if (messageIndex > listener.mEventFilter)
                    {
                        listener.mEventFilter = messageIndex;
                        listener.mFunction(messageDataT);
                    }
                });
        }
    };
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
        if(!dispatcher)
            dispatcher = std::make_unique<tDispatcher<T>>();
        else
        {
            if (dispatcher->mTypeIndex != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        ++mLastPostedMessageIndex;
        mEventsWriting.emplace_back(std::forward<T>(messageData), dispatcher.get());
    }
    template<class T> [[nodiscard]] cRegisteredID registerListener(const std::string& endPointID, std::function<void(const T&)> listener)
    {
        auto& dispatcher = mDispatchers[endPointID];
        if (!dispatcher)
            dispatcher = std::make_unique<tDispatcher<T>>();
        auto dispatcherT = dynamic_cast<tDispatcher<T>*>(dispatcher.get());
        if(!dispatcherT)
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



namespace MessageSystemTests
{

TEST(message_system, single_listen_post_receive)
{
    cMessageCenter messageCenter;

    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener<std::string>(
        "test.a.b.c", 
        [&](const std::string& message) 
        {
            EXPECT_EQ(message, "hello world");
            ++numberOfMessagesReceived;
        });

    messageCenter.post("test.a.b.c", "hello world"s);

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 1);
}

TEST(message_system, wrong_post_type)
{
    cMessageCenter messageCenter;
    auto listenerID = messageCenter.registerListener<std::string>(
        "test.a.b.c",
        [&](const std::string& message)
        {
        });
    ASSERT_THROW(messageCenter.post("test.a.b.c", 42), std::runtime_error);
}

TEST(message_system, wrong_listen_type)
{
    cMessageCenter messageCenter;
    messageCenter.post("test.a.b.c", "hello world"s);
    ASSERT_THROW(auto listenerID = messageCenter.registerListener<int>(
        "test.a.b.c",
        [&](int message)
        {
        }), std::runtime_error);
}


} // namespace MessageSystemTests
