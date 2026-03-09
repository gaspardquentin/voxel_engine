#include "voxel_engine/server/chat.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <stdexcept>


//History Ring buffer
class ChatHistory {
    size_t m_history_length;
    size_t m_count = 0;
    size_t m_head = 0;
    size_t m_first = 0;
    std::vector<voxeng::server::Message> m_messages;
public:
    ChatHistory(size_t history_length): m_history_length(history_length), m_messages(history_length) {
    }

    void pushMessage(voxeng::server::Message&& message) {
        m_messages[m_head] = message;

        if (m_count == m_history_length) {
            m_first = (m_first + 1) % m_history_length;
        } else {
            ++m_count;
        }

        m_head = (m_head + 1) % m_history_length;
    }

    size_t size() const {
        return m_count;
    }

    const voxeng::server::Message& getMessage(size_t id) const {
        if (m_count == 0) {
            throw std::runtime_error("Empty history");
        }

        size_t index = (id + m_first) % m_history_length;
        return m_messages[index];
    }

    const voxeng::server::Message& getLastMessage() const {
        if (m_count == 0) {
            throw std::runtime_error("Empty history");
        }

        size_t index = (m_head + m_history_length - 1) % m_history_length;
        return m_messages[index];
    }

    std::vector<voxeng::server::Message> getNLastMessages(size_t n) const {
        if (n > m_count) {
            throw std::runtime_error("Not enough messages to retrieve."); //TODO: maybe a runtime error is a bit too much here (warning might be better)
        }

        std::vector<voxeng::server::Message> ret;
        for (size_t i = 0; i < n; i++) {
            ret.push_back(getMessage(m_count - 1 - i));
        }
        return ret;
    }

    const voxeng::server::Message& getFirstMessage() {
        if (m_count == 0) {
            throw std::runtime_error("Empty history");
        }

        return m_messages[m_first];
    }

};


class voxeng::server::Chat::Impl {
public:
    Impl(size_t history_length): m_history(history_length) {}
    ChatHistory m_history;
};


voxeng::server::Chat::Chat(size_t history_length):
    m_impl(std::make_unique<voxeng::server::Chat::Impl>(history_length)) {
}

voxeng::server::Chat::~Chat() = default;

void voxeng::server::Chat::sendMessage(std::string sender_id, std::string content) {
    if (content.length() == 0) {
        return;
    }

    m_impl->m_history.pushMessage({
        sender_id,
        content,
        std::chrono::system_clock::now()
    });
}

//TODO: maybe remove this
std::vector<voxeng::server::Message> voxeng::server::Chat::getAllMessages() const {
    return m_impl->m_history.getNLastMessages(m_impl->m_history.size());
}


std::vector<voxeng::server::Message> voxeng::server::Chat::getRecentMessages(size_t count) const {
    //TODO: implement this
    return m_impl->m_history.getNLastMessages(count);
}

