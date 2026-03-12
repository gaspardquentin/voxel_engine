#pragma once

#include "voxel_engine/message.h"
#include "voxel_engine/user.h"
#include <memory>
#include <string>

namespace voxeng::server {

class Chat {
public:
    Chat(size_t history_length);
    ~Chat();

    void sendMessage(UserProfile sender, std::string content);

    std::vector<Message> getAllMessages() const;
    std::vector<Message> getRecentMessages(size_t count) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}
