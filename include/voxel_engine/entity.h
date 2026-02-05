#pragma once

#include "math_utils.h"
#include "mesh.h"
#include <memory>

namespace voxeng {

struct Entity {
    std::shared_ptr<Mesh> mesh;
    Vec3 position{0.0f, 0.0f, 0.0f};
    Vec3 rotation{0.0f, 0.0f, 0.0f};
    Vec3 scale{1.0f, 0.0f, 0.0f}; //TODO: a vec3 ? What about a simple float ?
};

}
