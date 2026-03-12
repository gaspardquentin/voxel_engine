#include "voxel_engine/chat_history.h"
#include <stdexcept>

namespace voxeng {

ChatHistory::ChatHistory(size_t history_length)
    : m_history_length(history_length), m_messages(history_length) {}

void ChatHistory::pushMessage(Message&& message) {
    m_messages[m_head] = std::move(message);

    if (m_count == m_history_length) {
        m_first = (m_first + 1) % m_history_length;
    } else {
        ++m_count;
    }

    m_head = (m_head + 1) % m_history_length;
}

size_t ChatHistory::size() const {
    return m_count;
}

const Message& ChatHistory::getMessage(size_t id) const {
    if (m_count == 0) {
        throw std::runtime_error("Empty history");
    }

    size_t index = (id + m_first) % m_history_length;
    return m_messages[index];
}

const Message& ChatHistory::getLastMessage() const {
    if (m_count == 0) {
        throw std::runtime_error("Empty history");
    }

    size_t index = (m_head + m_history_length - 1) % m_history_length;
    return m_messages[index];
}

const Message& ChatHistory::getFirstMessage() const {
    if (m_count == 0) {
        throw std::runtime_error("Empty history");
    }

    return m_messages[m_first];
}

std::vector<Message> ChatHistory::getNLastMessages(size_t n) const {
    if (n > m_count) {
        throw std::runtime_error("Not enough messages to retrieve.");
    }

    std::vector<Message> ret;
    for (size_t i = n; i > 0; i--) {
        ret.push_back(getMessage(m_count - i));
    }
    return ret;
}

std::vector<Message> ChatHistory::getAllMessages() const {
    return getNLastMessages(m_count);
}

}
