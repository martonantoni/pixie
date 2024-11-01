#include "messaging_tests_common.h"


namespace MessageSystemTests
{

//template<size_t... N, typename  Tuple>
//auto tTuplePrefixHelper(std::index_sequence<N...>, const Tuple&)
//{
//    return std::tuple<std::tuple_element_t<N, Tuple>...>{};
//}
//
//template<size_t N, typename Tuple> using tTuplePrefix = 
//    decltype(tTuplePrefixHelper(std::make_index_sequence<N>{}, std::declval<Tuple>()));
//
//template<size_t N, typename  Tuple>
//auto tPrefixTakerFunctionHelper(const Tuple& tuple)
//{
//    return []<size_t... I>(std::index_sequence<I...>)
//    {
//        return std::function<void(std::tuple_element_t<I, Tuple>...)>{};
//    }(std::make_index_sequence<N>{});
//}
//
//template<size_t N, typename  Tuple> using tPrefixTakerFunction =
//    decltype(tPrefixTakerFunctionHelper<N>(std::declval<Tuple>()));
//
//template<size_t... N, typename  Tuple>
//auto tMessageListenersHelper(std::index_sequence<N...>, const Tuple&)
//{
//    return std::variant<tPrefixTakerFunction<N, Tuple>...>{};
//}
//
//template<class Tuple> using tMessageListeners = 
//    decltype(tMessageListenersHelper(std::make_index_sequence<std::tuple_size_v<Tuple> + 1>{}, std::declval<Tuple>()));

//TEST(message_system, tuple_prefix)
//{
//    using T = std::tuple<int, double, std::string, char>;
//
//    static_assert(std::is_same_v<tTuplePrefix<1, T>, std::tuple<int>>);
//    static_assert(std::is_same_v<tTuplePrefix<2, T>, std::tuple<int, double>>);
//    static_assert(std::is_same_v<tTuplePrefix<3, T>, std::tuple<int, double, std::string>>);
//    static_assert(std::is_same_v<tTuplePrefix<4, T>, std::tuple<int, double, std::string, char>>);
//
//    // 0 must be empty tuple:
//    static_assert(std::is_same_v<tTuplePrefix<0, T>, std::tuple<>>);
//}
//
//TEST(message_system, prefix_taker_function)
//{
//    using T = std::tuple<int, double, std::string, char>;
//
//    static_assert(std::is_same_v<tPrefixTakerFunction<1, T>, std::function<void(int)>>);
//    static_assert(std::is_same_v<tPrefixTakerFunction<2, T>, std::function<void(int, double)>>);
//    static_assert(std::is_same_v<tPrefixTakerFunction<3, T>, std::function<void(int, double, std::string)>>);
//    static_assert(std::is_same_v<tPrefixTakerFunction<4, T>, std::function<void(int, double, std::string, char)>>);
//
//    // 0 must be empty function:
//    static_assert(std::is_same_v<tPrefixTakerFunction<0, T>, std::function<void()>>);
//}
//
//TEST(message_system, message_dispatchers_type)
//{
//    using T = std::tuple<int, double, std::string, char>;
//
//    static_assert(std::is_same_v<tMessageListeners<T>, std::variant<
//        std::function<void()>,
//        std::function<void(int)>,
//        std::function<void(int, double)>,
//        std::function<void(int, double, std::string)>,
//        std::function<void(int, double, std::string, char)>>>);
//}


template<class TUP> struct tDispatcher
{
    //    using cFunctions = std::variant
};


TEST(message_system, single_listen_post_receive)
{
    cMessageCenter messageCenter;

    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener2(
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

    messageCenter.post("test", "hello world"s);
    ASSERT_THROW(messageCenter.post("test", 33), std::runtime_error);
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

    auto listenerID = messageCenter.registerListener<std::string>(
        "test",
        [&](const std::string& message)
        {
        });
    ASSERT_THROW(auto listenerID = messageCenter.registerListener<int>(
        "test",
        [&](int message)
        {
        }), std::runtime_error);
}

TEST(message_system, post_before_listener_added_is_not_delivered)
{
    cMessageCenter messageCenter;
    messageCenter.post("test.a.b.c", 1);
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener<int>(
        "test.a.b.c",
        [&](int message)
        {
            EXPECT_EQ(message, 2);
            ++numberOfMessagesReceived;
        });
    messageCenter.post("test.a.b.c", 2);

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 1);
}

TEST(message_system, registering_from_listeners_gets_later_messages_delivered)
{
    cMessageCenter messageCenter;

    cRegisteredID secondListenerID;
    auto firstListenerID = messageCenter.registerListener<int>(
        "test.a.b.c",
        [&](int message)
        {
            secondListenerID = messageCenter.registerListener<int>(
                "test.d.e.f",
                [&](int message)
                {
                    EXPECT_EQ(message, 3);
                });
        });

    messageCenter.post("test.d.e.f", 1);  // must not get delivered
    messageCenter.post("test.a.b.c", 2);  // delivered, listener registers another listener
    messageCenter.post("test.d.e.f", 3);  // must get delivered

    messageCenter.dispatch();
}

TEST(message_system, listening_with_void)
{
    cMessageCenter messageCenter;
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener(
        "test",
        [&]()
        {
            ++numberOfMessagesReceived;
        });
    messageCenter.post("test", 1);
    messageCenter.post("test", 2);
    messageCenter.post("test", 3);

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);
}

TEST(message_system, void_end_points)
{
    cMessageCenter messageCenter;
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener<void>(
        "test",
        [&]()
        {
            ++numberOfMessagesReceived;
        });
    messageCenter.post("test");
    messageCenter.post("test");
    messageCenter.post("test");

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);
}

TEST(message_system, order_kept_with_different_endpoints)
{
    cMessageCenter messageCenter;
    std::vector<int> messagesReceived;
    auto listenerID1 = messageCenter.registerListener<int>(
        "test1",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    auto listenerID2 = messageCenter.registerListener<int>(
        "test2",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    auto listenerID3 = messageCenter.registerListener<int>(
        "test3",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    messageCenter.post("test3", 1);
    messageCenter.post("test1", 2);
    messageCenter.post("test2", 3);
    messageCenter.post("test3", 4);
    messageCenter.post("test1", 5);
    messageCenter.post("test2", 6);

    messageCenter.dispatch();

    EXPECT_EQ(messagesReceived.size(), 6);
    for(int i=1;i<=6;++i)
    {
        EXPECT_EQ(messagesReceived[i-1], i);
    }

}

TEST(message_system, unregister)
{
    cMessageCenter messageCenter;
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener<int>(
        "test",
        [&](int message)
        {
            ++numberOfMessagesReceived;
        });
    for(int i=1;i<=3;++i)
    {
        messageCenter.post("test", i);
    }

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);

    listenerID.Unregister();

    messageCenter.post("test", 4);
    messageCenter.post("test", 5);
    messageCenter.post("test", 6);

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3); // meaning no new messages were received
}

TEST(message_system, unregister_in_handler)
{
    cMessageCenter messageCenter;
    int numberOfMessagesReceived = 0;
    cRegisteredID listenerID;
    std::vector<int> messagesReceived;
    listenerID = messageCenter.registerListener<int>(
        "test",
        [&](int message)
        {
            messagesReceived.push_back(message);
            ++numberOfMessagesReceived;
            if (numberOfMessagesReceived == 3)
            {
                listenerID.Unregister();
            }
        });
    for (int i = 1; i <= 6; ++i)
    {
        messageCenter.post("test", i);
    }

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);
    for(int i=1;i<=3;++i)
    {
        EXPECT_EQ(messagesReceived[i-1], i);
    }
}

TEST(message_system, unregister_in_handler_many_listeners)
{
    cMessageCenter messageCenter;
    std::vector<cRegisteredID> listenerIDs;
    std::array<std::vector<int>,10> messagesReceived;
    for (int i = 0; i < 10; ++i)
    {
        listenerIDs.push_back(messageCenter.registerListener<int>(
            "test",
            [i, numberOfMessagesReceived = 0, &listenerIDs, &messagesReceived](int message) mutable
            {
                messagesReceived[i].push_back(message);
                ++numberOfMessagesReceived;
                if (numberOfMessagesReceived == (i + 5) % 10 + 1)
                {
                    listenerIDs[i].Unregister();
                }
            }));
    }
    for (int i = 1; i <= 20; ++i)
    {
        messageCenter.post("test", i);
    }

    messageCenter.dispatch();
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(messagesReceived[i].size(), (i + 5) % 10 + 1) << "i=" << i;
        for(int j=1;j<=messagesReceived[i].size();++j)
        {
            EXPECT_EQ(messagesReceived[i][j-1], j);
        }
    }
}

TEST(message_system, not_leaking_messages)
{
    cMessageCenter messageCenter;
    int numberOfAliveMessages = 0;
    int numberOfMessagesReceived = 0;
    struct cTestMessage
    {
        int& mMessageCount;
        cTestMessage(int& messageCount): mMessageCount(messageCount) { ++mMessageCount; }
        ~cTestMessage() { --mMessageCount; }
        cTestMessage(const cTestMessage& src): mMessageCount(src.mMessageCount) { ++mMessageCount; }
    };
    auto listenerID = messageCenter.registerListener<cTestMessage>(
        "test",
        [&](const cTestMessage&)
        {
            ++numberOfMessagesReceived;
        });
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            messageCenter.post("test", cTestMessage(numberOfAliveMessages));
        }
        messageCenter.dispatch();
    }
    EXPECT_EQ(numberOfAliveMessages, 0);
    EXPECT_EQ(numberOfMessagesReceived, 50);
}

TEST(message_system, message_sent_from_listener)
{
    cMessageCenter messageCenter;
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter.registerListener<int>(
        "test",
        [&](int message)
        {
            ++numberOfMessagesReceived;
            if (numberOfMessagesReceived < 10)
            {
                messageCenter.post("test", message + 1);
            }
        });
    messageCenter.post("test", 1);

    messageCenter.dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 10);
}

} // namespace MessageSystemTests
