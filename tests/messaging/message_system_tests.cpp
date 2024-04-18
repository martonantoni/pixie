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
    auto listenerID = messageCenter.registerListener<int>(
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

} // namespace MessageSystemTests
