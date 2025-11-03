#include "messaging_tests_common.h"


namespace MessageSystemTests
{

TEST(message_sequencing, nv_listener_gets_message_index)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    int numberOfMessagesReceived = 0;
    auto listener = messageCenter->registerListener(
        "test.seq_test",
        [&](cMessageSequencingID sequencingID, int message)
        {
            EXPECT_EQ(message, 12);
            ++numberOfMessagesReceived;
        });
    auto normalListener = messageCenter->registerListener(
        "test.seq_test",
        [&](int message)
        {
            EXPECT_EQ(message, 12);
            ++numberOfMessagesReceived;
        });
    messageCenter->post("test.seq_test", 12);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 2);
}

TEST(message_sequencing, v_listener_gets_message_index)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    auto listener = messageCenter->registerListener(
        "test.seq_test",
        [&](cMessageSequencingID sequencingID)
        {
            ++numberOfMessagesReceived;
        });
    auto normalListener = messageCenter->registerListener(
        "test.seq_test",
        [&](int message)
        {
            EXPECT_EQ(message, 12);
            ++numberOfMessagesReceived;
        });

    messageCenter->post("test.seq_test", 12);
    messageCenter->dispatch();
    EXPECT_EQ(numberOfMessagesReceived, 2);
}

TEST(message_sequencing, sequence_starting_message_delivered)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    int numberOfMessagesReceived = 0;
    auto listener = messageCenter->registerListener(
        "test.test_seq",
        [&](int a) 
        { 
            if (numberOfMessagesReceived < 2)
            {
                auto expected = std::array{ 12, 56 }[numberOfMessagesReceived];
                EXPECT_EQ(a, expected);
            }
            ++numberOfMessagesReceived;
        });

    cMessageSequence sequence = messageCenter->sequence("test.test_seq", 12)
        .on("test.test_seq.reply_a", [](int a) { EXPECT_EQ(a, 34); })
        .on("test.test_seq.reply_b", [](const std::string& b) { EXPECT_STREQ(b.c_str(), "hello"); });


    cMessageSequence sequence2 = messageCenter->sequence("test.test_seq", 56);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 2);
}

void responseMessageDeliveredTest(bool usePost)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    std::unordered_map<std::string, int> repliesReceived;
    int numberOfMessagesReceived = 0;

    auto listener = messageCenter->registerListener(
        "test.test_seq",
        [&](cMessageSequencingID idx, int a)
        {
            if (numberOfMessagesReceived < 2)
            {
                auto expected = std::array{ 12, 56 }[numberOfMessagesReceived];
                EXPECT_EQ(a, expected);
                auto replyDestination = std::array{ "test.test_seq.reply_a"s, "test.test_seq.reply_c"s }[numberOfMessagesReceived];
                if(usePost)
                    messageCenter->postResponse(idx.mThisMessage, replyDestination, std::array{ 34, 78 }[numberOfMessagesReceived]);
                else
                    messageCenter->sendResponse(idx.mThisMessage, replyDestination, std::array{ 34, 78 }[numberOfMessagesReceived]);
            }
            ++numberOfMessagesReceived;
        });
    cMessageSequence sequence = messageCenter->sequence("test.test_seq", 12)
        .on("test.test_seq.reply_a", [&](int a) { EXPECT_EQ(a, 34); ++repliesReceived["reply_a"]; })
        .on("test.test_seq.reply_b", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "hello"); ++repliesReceived["reply_b"]; });
    cMessageSequence sequence2 = messageCenter->sequence("test.test_seq", 56)
        .on("test.test_seq.reply_c", [&](int a) { EXPECT_EQ(a, 78); ++repliesReceived["reply_c"]; })
        .on("test.test_seq.reply_d", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "world"); ++repliesReceived["reply_d"]; });

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 2);

    EXPECT_EQ(repliesReceived["reply_a"], 1);
    EXPECT_EQ(repliesReceived["reply_b"], 0);
    EXPECT_EQ(repliesReceived["reply_c"], 1);
    EXPECT_EQ(repliesReceived["reply_d"], 0);
}

TEST(message_sequencing, response_message_delivered)
{
    responseMessageDeliveredTest(true);
}
TEST(message_sequencing, send_response_message_delivered)
{
    responseMessageDeliveredTest(false);
}

void filterOutTest(bool usePost)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    std::unordered_map<std::string, int> repliesReceived;
    int numberOfMessagesReceived = 0;

    auto listener = messageCenter->registerListener(
        "test.test_seq",
        [&](cMessageSequencingID idx, int a)
        {
            if (numberOfMessagesReceived < 2)
            {
                auto expected = std::array{ 12, 56 }[numberOfMessagesReceived];
                EXPECT_EQ(a, expected);
                auto replyDestination = std::array{ "test.test_seq.reply_a", "test.test_seq.reply_c" }[numberOfMessagesReceived];
                if(usePost)
                    messageCenter->postResponse(idx.mThisMessage, replyDestination, std::array{ 34, 78 }[numberOfMessagesReceived]);
                else
                    messageCenter->sendResponse(idx.mThisMessage, replyDestination, std::array{ 34, 78 }[numberOfMessagesReceived]);
            }
            ++numberOfMessagesReceived;
        });

    messageCenter->post("test.test_seq.reply_a", 23);  // must not get delivered
    messageCenter->post("test.test_seq.reply_b", "alma"s);  // must not get delivered
    messageCenter->post("test.test_seq.reply_c", 33); // must not get delivered
    messageCenter->post("test.test_seq.reply_d", "alma"s);  // must not get delivered

    cMessageSequence sequence = messageCenter->sequence("test.test_seq", 12)
        .on("test.test_seq.reply_a", [&](int a) { EXPECT_EQ(a, 34); ++repliesReceived["reply_a"]; })
        .on("test.test_seq.reply_b", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "hello"); ++repliesReceived["reply_b"]; });
    cMessageSequence sequence2 = messageCenter->sequence("test.test_seq", 56)
        .on("test.test_seq.reply_c", [&](int a) { EXPECT_EQ(a, 78); ++repliesReceived["reply_c"]; })
        .on("test.test_seq.reply_d", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "world"); ++repliesReceived["reply_d"]; });

    messageCenter->post("test.test_seq.reply_a", 99);  // must not get delivered
    messageCenter->post("test.test_seq.reply_b", "alma"s);  // must not get delivered
    messageCenter->post("test.test_seq.reply_c", 100); // must not get delivered
    messageCenter->post("test.test_seq.reply_d", "alma"s);  // must not get delivered

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 2);
    EXPECT_EQ(repliesReceived["reply_a"], 1);
    EXPECT_EQ(repliesReceived["reply_b"], 0);
    EXPECT_EQ(repliesReceived["reply_c"], 1);
    EXPECT_EQ(repliesReceived["reply_d"], 0);
}

TEST(message_sequencing, filtering_out_messages)
{
    filterOutTest(true);
}
TEST(message_sequencing, send_filtering_out_messages)
{
    filterOutTest(false);
}
TEST(message_sequencing, abandoning_sequence)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    std::unordered_map<std::string, int> repliesReceived;
    int numberOfMessagesReceived = 0;
    auto listener = messageCenter->registerListener(
        "test.test_seq",
        [&](cMessageSequencingID idx, int a)
        {
            if (numberOfMessagesReceived < 2)
            {
                auto expected = std::array{ 12, 56 }[numberOfMessagesReceived];
                EXPECT_EQ(a, expected);
                auto replyDestination = std::array{ "test.test_seq.reply_a", "test.test_seq.reply_c" }[numberOfMessagesReceived];
                messageCenter->postResponse(idx.mThisMessage, replyDestination, std::array{ 34, 78 }[numberOfMessagesReceived]);
            }
            ++numberOfMessagesReceived;
        });
    {
        cMessageSequence sequence = messageCenter->sequence("test.test_seq", 12)
            .on("test.test_seq.reply_a", [&](int a) { EXPECT_EQ(a, 34); ++repliesReceived["reply_a"]; })
            .on("test.test_seq.reply_b", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "hello"); ++repliesReceived["reply_b"]; });
        cMessageSequence sequence2 = messageCenter->sequence("test.test_seq", 56)
            .on("test.test_seq.reply_c", [&](int a) { EXPECT_EQ(a, 78); ++repliesReceived["reply_c"]; })
            .on("test.test_seq.reply_d", [&](const std::string& b) { EXPECT_STREQ(b.c_str(), "world"); ++repliesReceived["reply_d"]; });
    } // sequences go out of scope here
    messageCenter->dispatch();
    EXPECT_EQ(numberOfMessagesReceived, 2);
    EXPECT_EQ(repliesReceived["reply_a"], 0);
    EXPECT_EQ(repliesReceived["reply_b"], 0);
    EXPECT_EQ(repliesReceived["reply_c"], 0);
    EXPECT_EQ(repliesReceived["reply_d"], 0);
}

TEST(message_sequencing, post_vs_send)
{
    // tests that post is asynchronous and send is synchronous

    auto messageCenter = std::make_shared<cMessageCenter>();

    std::array<bool, 2> postTargetReceivedMessage = { false, false };
    std::array<bool, 2> sendTargetReceivedMessage = { false, false };

    auto listener = messageCenter->registerListener(
        "test.test_seq",
        [&](cMessageSequencingID idx, int a)
        {
            messageCenter->postResponse(idx.mThisMessage, "test.post_target", 1);
            for(auto& v : sendTargetReceivedMessage)
                EXPECT_FALSE(v);
            for (auto& v : postTargetReceivedMessage)
                EXPECT_FALSE(v);
            messageCenter->sendResponse(idx.mThisMessage, "test.send_target", 2);
            for (auto& v : sendTargetReceivedMessage)
                EXPECT_TRUE(v);
            for (auto& v : postTargetReceivedMessage)
                EXPECT_FALSE(v);
        });

    auto generalPostListener = messageCenter->registerListener(
        "test.post_target",
        [&](int a)
        {
            postTargetReceivedMessage[0] = true;
        });

    auto sendListener = messageCenter->registerListener(
        "test.send_target",
        [&](int a)
        {
            sendTargetReceivedMessage[0] = true;
        });

    cMessageSequence sequence = messageCenter->sequence("test.test_seq", 12)
        .on("test.post_target", [&](int a) { EXPECT_EQ(a, 1); postTargetReceivedMessage[1] = true; })
        .on("test.send_target", [&](int a) { EXPECT_EQ(a, 2); sendTargetReceivedMessage[1] = true; });

    messageCenter->dispatch();

    EXPECT_TRUE(sendTargetReceivedMessage[0]);
    EXPECT_TRUE(sendTargetReceivedMessage[1]);
    EXPECT_TRUE(postTargetReceivedMessage[0]);
    EXPECT_TRUE(postTargetReceivedMessage[1]);
}

TEST(message_system, single_listen_post_receive)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test.a.b.c", 
        [&](const std::string& message) 
        {
            EXPECT_EQ(message, "hello world");
            ++numberOfMessagesReceived;
        });

    messageCenter->post("test.a.b.c", "hello world"s);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 1);
}

TEST(message_system, multi_param_all_listening)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test.a.b.c",
        [&](int a, const std::string& b, int c)
        {
            EXPECT_EQ(a, 1);
            EXPECT_STREQ(b.c_str(), "alma");
            EXPECT_EQ(c, 3);
            ++numberOfMessagesReceived;
        });

    messageCenter->post("test.a.b.c", 1, "alma"s, 3);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 1);
}

// TODO: make this into an error test case

//TEST(message_system, multi_param_partial_listening)
//{
//    auto messageCenter = std::make_shared<cMessageCenter>();
//
//    int numberOfMessagesReceived = 0;
//    auto listenerID = messageCenter->registerListener(
//        "test.a.b.c",
//        [&](int a, const std::string& b)
//        {
//            EXPECT_EQ(a, 1);
//            EXPECT_STREQ(b.c_str(), "alma");
//            ++numberOfMessagesReceived;
//        });
//
//    messageCenter->post("test.a.b.c", 1, "alma"s, 3);
//
//    messageCenter->dispatch();
//
//    EXPECT_EQ(numberOfMessagesReceived, 1);
//
//}

TEST(message_system, wrong_post_type)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    auto listenerID = messageCenter->registerListener(
        "test.a.b.c",
        [&](const std::string& message)
        {
        });
    ASSERT_THROW(messageCenter->post("test.a.b.c", 42), std::runtime_error);

    messageCenter->post("test", "hello world"s);
    ASSERT_THROW(messageCenter->post("test", 33), std::runtime_error);
}

TEST(message_system, wrong_listen_type)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    messageCenter->post("test.a.b.c", "hello world"s);
    ASSERT_THROW(auto listenerID = messageCenter->registerListener(
        "test.a.b.c",
        [&](int message)
        {
        }), std::runtime_error);

    auto listenerID = messageCenter->registerListener(
        "test",
        [&](const std::string& message)
        {
        });
    ASSERT_THROW(auto listenerID = messageCenter->registerListener(
        "test",
        [&](int message)
        {
        }), std::runtime_error);
}

TEST(message_system, post_before_listener_added_is_not_delivered)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    messageCenter->post("test.a.b.c", 1);
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test.a.b.c",
        [&](int message)
        {
            EXPECT_EQ(message, 2);
            ++numberOfMessagesReceived;
        });
    messageCenter->post("test.a.b.c", 2);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 1);
}

TEST(message_system, registering_from_listeners_gets_later_messages_delivered)
{
    auto messageCenter = std::make_shared<cMessageCenter>();

    cRegisteredID secondListenerID;
    auto firstListenerID = messageCenter->registerListener(
        "test.a.b.c",
        [&](int message)
        {
            secondListenerID = messageCenter->registerListener(
                "test.d.e.f",
                [&](int message)
                {
                    EXPECT_EQ(message, 3);
                });
        });

    messageCenter->post("test.d.e.f", 1);  // must not get delivered
    messageCenter->post("test.a.b.c", 2);  // delivered, listener registers another listener
    messageCenter->post("test.d.e.f", 3);  // must get delivered

    messageCenter->dispatch();
}

TEST(message_system, listening_with_void)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test",
        [&]()
        {
            ++numberOfMessagesReceived;
        });
    messageCenter->post("test", 1);
    messageCenter->post("test", 2);
    messageCenter->post("test", 3);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);
}

TEST(message_system, void_end_points)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test",
        [&]()
        {
            ++numberOfMessagesReceived;
        });
    messageCenter->post("test");
    messageCenter->post("test");
    messageCenter->post("test");

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);
}

TEST(message_system, order_kept_with_different_endpoints)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    std::vector<int> messagesReceived;
    auto listenerID1 = messageCenter->registerListener(
        "test1",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    auto listenerID2 = messageCenter->registerListener(
        "test2",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    auto listenerID3 = messageCenter->registerListener(
        "test3",
        [&](int message)
        {
            messagesReceived.push_back(message);
        });
    messageCenter->post("test3", 1);
    messageCenter->post("test1", 2);
    messageCenter->post("test2", 3);
    messageCenter->post("test3", 4);
    messageCenter->post("test1", 5);
    messageCenter->post("test2", 6);

    messageCenter->dispatch();

    ASSERT_EQ(messagesReceived.size(), 6);
    for(int i=1;i<=6;++i)
    {
        EXPECT_EQ(messagesReceived[i-1], i);
    }

}

TEST(message_system, unregister)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test",
        [&](int message)
        {
            ++numberOfMessagesReceived;
        });
    for(int i=1;i<=3;++i)
    {
        messageCenter->post("test", i);
    }

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3);

    listenerID.Unregister();

    messageCenter->post("test", 4);
    messageCenter->post("test", 5);
    messageCenter->post("test", 6);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 3); // meaning no new messages were received
}

TEST(message_system, unregister_in_handler)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    cRegisteredID listenerID;
    std::vector<int> messagesReceived;
    listenerID = messageCenter->registerListener(
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
        messageCenter->post("test", i);
    }

    messageCenter->dispatch();

    ASSERT_EQ(numberOfMessagesReceived, 3);
    for(int i=1;i<=3;++i)
    {
        EXPECT_EQ(messagesReceived[i-1], i);
    }
}

TEST(message_system, unregister_in_handler_many_listeners)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    std::vector<cRegisteredID> listenerIDs;
    std::array<std::vector<int>,10> messagesReceived;
    for (int i = 0; i < 10; ++i)
    {
        listenerIDs.push_back(messageCenter->registerListener(
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
        messageCenter->post("test", i);
    }

    messageCenter->dispatch();
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
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfAliveMessages = 0;
    int numberOfMessagesReceived = 0;
    struct cTestMessage
    {
        int& mMessageCount;
        cTestMessage(int& messageCount): mMessageCount(messageCount) { ++mMessageCount; }
        ~cTestMessage() { --mMessageCount; }
        cTestMessage(const cTestMessage& src): mMessageCount(src.mMessageCount) { ++mMessageCount; }
    };
    auto listenerID = messageCenter->registerListener(
        "test",
        [&](const cTestMessage&)
        {
            ++numberOfMessagesReceived;
        });
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            messageCenter->post("test", cTestMessage(numberOfAliveMessages));
        }
        messageCenter->dispatch();
    }
    EXPECT_EQ(numberOfAliveMessages, 0);
    EXPECT_EQ(numberOfMessagesReceived, 50);
}

TEST(message_system, message_sent_from_listener)
{
    auto messageCenter = std::make_shared<cMessageCenter>();
    int numberOfMessagesReceived = 0;
    auto listenerID = messageCenter->registerListener(
        "test",
        [&](int message)
        {
            ++numberOfMessagesReceived;
            if (numberOfMessagesReceived < 10)
            {
                messageCenter->post("test", message + 1);
            }
        });
    messageCenter->post("test", 1);

    messageCenter->dispatch();

    EXPECT_EQ(numberOfMessagesReceived, 10);
}

} // namespace MessageSystemTests



//mCompileSequence = themessageCenter->sequence("ready.compile", source).
//response("ready.compile.failed", [](auto& errors) { handleCompileErrors(errors); }).
//response("ready.compile.success", [](auto& program) { handleCompileSuccess(program); });
