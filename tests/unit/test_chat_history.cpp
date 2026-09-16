#include <gtest/gtest.h>
#include "voxel_engine/chat_history.h"
#include "voxel_engine/message.h"
#include "voxel_engine/user.h"
#include <chrono>
#include <string>

using namespace voxeng;

static Message makeMsg(const std::string& content) {
    return Message{
        UserProfile{1, "TestUser"},
        content,
        std::chrono::system_clock::now()
    };
}

TEST(ChatHistory, EmptyHistory) {
    ChatHistory history(10);
    EXPECT_EQ(history.size(), 0u);
    EXPECT_TRUE(history.getAllMessages().empty());
}

TEST(ChatHistory, SingleMessage) {
    ChatHistory history(10);
    history.pushMessage(makeMsg("hello"));
    EXPECT_EQ(history.size(), 1u);
    EXPECT_EQ(history.getLastMessage().content, "hello");
    EXPECT_EQ(history.getFirstMessage().content, "hello");
}

TEST(ChatHistory, MultipleMessages) {
    ChatHistory history(10);
    history.pushMessage(makeMsg("first"));
    history.pushMessage(makeMsg("second"));
    history.pushMessage(makeMsg("third"));

    EXPECT_EQ(history.size(), 3u);
    EXPECT_EQ(history.getFirstMessage().content, "first");
    EXPECT_EQ(history.getLastMessage().content, "third");

    auto all = history.getAllMessages();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0].content, "first");
    EXPECT_EQ(all[1].content, "second");
    EXPECT_EQ(all[2].content, "third");
}

TEST(ChatHistory, FillToCapacity) {
    ChatHistory history(5);
    for (int i = 0; i < 5; i++) {
        history.pushMessage(makeMsg("msg_" + std::to_string(i)));
    }

    EXPECT_EQ(history.size(), 5u);
    EXPECT_EQ(history.getFirstMessage().content, "msg_0");
    EXPECT_EQ(history.getLastMessage().content, "msg_4");
}

TEST(ChatHistory, OverflowWraparound) {
    ChatHistory history(3);
    history.pushMessage(makeMsg("a"));
    history.pushMessage(makeMsg("b"));
    history.pushMessage(makeMsg("c"));
    history.pushMessage(makeMsg("d"));
    history.pushMessage(makeMsg("e"));

    // Size stays at capacity
    EXPECT_EQ(history.size(), 3u);
    // Oldest surviving is "c"
    EXPECT_EQ(history.getFirstMessage().content, "c");
    EXPECT_EQ(history.getLastMessage().content, "e");

    auto all = history.getAllMessages();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0].content, "c");
    EXPECT_EQ(all[1].content, "d");
    EXPECT_EQ(all[2].content, "e");
}

TEST(ChatHistory, GetNLastMessages) {
    ChatHistory history(10);
    for (int i = 0; i < 5; i++) {
        history.pushMessage(makeMsg("msg_" + std::to_string(i)));
    }

    auto last2 = history.getNLastMessages(2);
    ASSERT_EQ(last2.size(), 2u);
    EXPECT_EQ(last2[0].content, "msg_3");
    EXPECT_EQ(last2[1].content, "msg_4");

    auto all = history.getNLastMessages(5);
    EXPECT_EQ(all.size(), 5u);
}

TEST(ChatHistory, GetNLastMessages_ThrowsIfTooMany) {
    ChatHistory history(10);
    history.pushMessage(makeMsg("one"));
    history.pushMessage(makeMsg("two"));
    history.pushMessage(makeMsg("three"));

    EXPECT_THROW(history.getNLastMessages(4), std::runtime_error);
}

TEST(ChatHistory, GetMessage_ThrowsOnEmpty) {
    ChatHistory history(10);
    EXPECT_THROW(history.getMessage(0), std::runtime_error);
}

TEST(ChatHistory, GetLastMessage_ThrowsOnEmpty) {
    ChatHistory history(10);
    EXPECT_THROW(history.getLastMessage(), std::runtime_error);
}

TEST(ChatHistory, GetFirstMessage_ThrowsOnEmpty) {
    ChatHistory history(10);
    EXPECT_THROW(history.getFirstMessage(), std::runtime_error);
}

TEST(ChatHistory, HeavyWraparound) {
    ChatHistory history(4);
    for (int i = 0; i < 100; i++) {
        history.pushMessage(makeMsg("msg_" + std::to_string(i)));
    }

    EXPECT_EQ(history.size(), 4u);

    auto all = history.getAllMessages();
    ASSERT_EQ(all.size(), 4u);
    EXPECT_EQ(all[0].content, "msg_96");
    EXPECT_EQ(all[1].content, "msg_97");
    EXPECT_EQ(all[2].content, "msg_98");
    EXPECT_EQ(all[3].content, "msg_99");
}

TEST(ChatHistory, SenderPreserved) {
    ChatHistory history(5);
    Message msg{UserProfile{42, "Gaspard"}, "hello world", std::chrono::system_clock::now()};
    history.pushMessage(std::move(msg));

    const auto& retrieved = history.getLastMessage();
    EXPECT_EQ(retrieved.sender.id, 42u);
    EXPECT_EQ(retrieved.sender.username, "Gaspard");
}
