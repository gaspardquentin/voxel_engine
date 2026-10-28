#pragma once


#include "voxel_engine/math_utils.h"
#include "voxel_engine/types.h"
#include <functional>
namespace voxeng::collision {

constexpr float EPSILON = 1e-4f;

/// Player bounding box size. The box's min corner is at the feet.
constexpr Vec3f PLAYER_SIZE{0.6f, 1.8f, 0.6f};
/// Eye position relative to the min corner of the player's box.
constexpr Vec3f PLAYER_EYE_OFFSET{0.3f, 1.6f, 0.3f};

/**
* @brief Returns whether the entity at position pos is colliding with some blocks in the world.
*
* @param pos the entity position in the world (min corner of its bounding box)
* @param size the size of the entity
* @param isSolid a function to call that returns whether a block at a specified position is solid or not.
*/
bool isColliding(const WorldCoord& pos, const Vec3f& size, const std::function<bool(Vec3i)>& isSolid);

}
