#pragma once

#include "entt/entt.hpp"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/server/entity_components.h"

namespace voxeng::server::movement_system {

inline void update(entt::registry& reg, float delta_time) {
    for (auto [entity, pos, vel] : reg.view<Position, Velocity>().each()) {
        pos.pos.x += vel.vel.x * delta_time;
        pos.pos.y += vel.vel.y * delta_time;
        pos.pos.z += vel.vel.z * delta_time;
    }
}

}
