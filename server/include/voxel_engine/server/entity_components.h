#pragma once


#include "voxel_engine/callbacks.h"
#include "voxel_engine/math_utils.h"
#include <string>

namespace voxeng::server {

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
