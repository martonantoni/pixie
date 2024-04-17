#include "event_tests_common.h"

class cMessageDataStorage
{
protected:
    static int mNextMessageDataIndex;
};

template<class T>
class tMessageDataStorage: public tSingleton<tMessageDataStorage<T>>, public cMessageDataStorage
{
    std::map<int, T> data;
public:
    int store(T&& event)
    {
        int index = mNextMessageDataIndex++;
        data[index] = std::move(event);
        return index; // return the index of the added event
    }

    const T& retrieve(int dataIndex) const
    {
        if (data.find(dataIndex) == data.end())
            throw std::out_of_range("Invalid index");

        return data.at(dataIndex);
    }

    void remove(int dataIndex)
    {
        if (data.find(dataIndex) == data.end())
            throw std::out_of_range("Invalid index");

        data.erase(dataIndex);
    }

};

int cMessageDataStorage::mNextMessageDataIndex = 0;

class cMessageCenter
{
    struct cListener
    {
        std::function<void(int)> mFunction;
        int mEventFilter = -1;
    };
    using cListeners = tSafeObjects<cListener>;
    struct cDispatcher: public cRegistrationHandler
    {
        std::type_index mTypeIndex;
        cListeners mListeners;
        virtual void Unregister(const cRegisteredID& RegisteredID, eCallbackType CallbackType) override
        {
            mListeners.Unregister(RegisteredID.GetID());
        }
        cDispatcher(const std::type_index& typeIndex): mTypeIndex(typeIndex) {}
    };
    std::unordered_map<std::string, std::unique_ptr<cDispatcher>> mDispatchers;
    struct cEvent
    {
        int mEventID;
        cListeners* mListeners;
    };
    std::vector<cEvent> mEventsWriting;
    std::vector<cEvent> mEventsReading;
public:
    template<class T> void post(const std::string& eventID, T&& event)
    {
        auto eventIndex = tMessageDataStorage<T>::get().store(std::forward<T>(event));
        auto& dispatcher = mDispatchers[eventID];
        if(!dispatcher)
        {
            dispatcher = std::make_unique<cDispatcher>(typeid(T));
        }
        else
        {
            if (dispatcher->mTypeIndex != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        mEventsWriting.emplace_back(eventIndex, &dispatcher->mListeners);
    }
    template<class T> [[nodiscard]] cRegisteredID registerListener(const std::string& eventID, std::function<void(const T&)> listener)
    {
        auto& dispatcher = mDispatchers[eventID];
        if (!dispatcher)
        {
            dispatcher = std::make_unique<cDispatcher>(typeid(T));
        }
        else
        {
            if (dispatcher->mTypeIndex != typeid(T))
                throw std::runtime_error("Wrong message type");
        }
        return cRegisteredID(dispatcher.get(), dispatcher->mListeners.Register([this, listener](int eventIndex)
            {
                listener(tMessageDataStorage<T>::get().retrieve(eventIndex));
            }));
    }
    void dispatch()
    {
        std::swap(mEventsReading, mEventsWriting);
        for (auto& event : mEventsReading)
        {
            event.mListeners->ForEach([&event](auto& listener)
                {
                    if (event.mEventID > listener.mEventFilter)
                    {
                        listener.mEventFilter = event.mEventID;
                        listener.mFunction(event.mEventID);
                    }
                });

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
