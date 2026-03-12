#pragma once

#include "voxel_engine/message.h"
#include <cstddef>
#include <vector>

namespace voxeng {

class ChatHistory {
    size_t m_history_length;
    size_t m_count = 0;
    size_t m_head = 0;
    size_t m_first = 0;
    std::vector<Message> m_messages;
public:
    ChatHistory(size_t history_length);

    void pushMessage(Message&& message);

    size_t size() const;

    const Message& getMessage(size_t id) const;
    const Message& getLastMessage() const;
    const Message& getFirstMessage() const;

    std::vector<Message> getNLastMessages(size_t n) const;
    std::vector<Message> getAllMessages() const;
};

}
