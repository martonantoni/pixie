#include "messaging_tests_common.h"


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
