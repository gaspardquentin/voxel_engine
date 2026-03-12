#include "voxel_engine/server/chat.h"
#include "voxel_engine/chat_history.h"
#include "voxel_engine/user.h"
#include <chrono>
#include <memory>


class voxeng::server::Chat::Impl {
public:
    Impl(size_t history_length): m_history(history_length) {}
    voxeng::ChatHistory m_history;
};


voxeng::server::Chat::Chat(size_t history_length):
    m_impl(std::make_unique<voxeng::server::Chat::Impl>(history_length)) {
}

voxeng::server::Chat::~Chat() = default;

void voxeng::server::Chat::sendMessage(voxeng::UserProfile sender, std::string content) {
    if (content.length() == 0) {
        return;
    }

    m_impl->m_history.pushMessage({
        sender,
        content,
        std::chrono::system_clock::now()
    });
}

std::vector<voxeng::Message> voxeng::server::Chat::getAllMessages() const {
    return m_impl->m_history.getAllMessages();
}


std::vector<voxeng::Message> voxeng::server::Chat::getRecentMessages(size_t count) const {
    return m_impl->m_history.getNLastMessages(count);
}
