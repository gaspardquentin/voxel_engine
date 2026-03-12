#pragma once

#include "voxel_engine/math_utils.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace voxeng {

using UserID = uint64_t;

struct UserProfile {
    UserID id;
    std::string username;
};

}
