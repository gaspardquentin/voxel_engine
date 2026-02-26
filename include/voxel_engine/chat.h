#pragma once

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

namespace voxeng {

struct Message {
    std::string sender_id;
    std::string content;
    std::chrono::system_clock::time_point timestamp;

    std::string getFormattedTime() const {
        auto t = std::chrono::system_clock::to_time_t(timestamp);
        std::tm* tm = std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(tm, "%H:%M:%S");

        return oss.str();
    }
};

class Chat {
public:
    Chat(size_t history_length);
    ~Chat();

    void sendMessage(std::string sender_id, std::string content);

    std::vector<Message> getAllMessages() const;
    std::vector<Message> getRecentMessages(size_t count) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}
