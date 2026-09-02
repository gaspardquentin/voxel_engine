#pragma once


#include "voxel_engine/math_utils.h"
#include "voxel_engine/types.h"
#include "voxel_engine/user.h"
#include <cstdint>
#include <string>

namespace voxeng::server {


struct PlayerData {
    UserProfile profile;
    uint8_t render_distance;
};

struct ChunkTracking {
    ChunkID last_chunk = {INT32_MAX, INT32_MAX};
};

struct Position {
    WorldCoord pos;
};

struct Velocity {
    Vec3f vel;
};

struct Model {
    std::string model_name;
};

}
