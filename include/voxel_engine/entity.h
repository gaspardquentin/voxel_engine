#pragma once

#include "voxel_engine/mesh.h"
#include <glm/glm.hpp>
#include <memory>

namespace voxeng {

struct Entity {
    std::shared_ptr<Mesh> mesh;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
};

}
