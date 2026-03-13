#pragma once

#include "voxel_engine/types.h"
#include <fstream>
#include <iostream>
#include <string>

namespace voxeng {

struct UserProfile {
    UserID id;
    std::string username;
};

}
