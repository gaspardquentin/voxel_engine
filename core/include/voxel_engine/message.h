#pragma once

#include "voxel_engine/user.h"
#include <string>
#include <chrono>
#include <iomanip>

namespace voxeng {


struct Message {
    UserProfile sender;
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

}
