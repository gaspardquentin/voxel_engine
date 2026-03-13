#pragma once

#include <cstdint>
#include "math_utils.h"

// Voxel types
using VoxelID = uint8_t;

// Coordinate types
//Note: doing this limits the amount of chunks of the engine, but this might be
//enough and makes the hashing function more efficient
//This might change in the future.
using ChunkID = Vec2T<int32_t>;
using ChunkCoord = Vec3u;
using WorldCoord = Vec3f;

// Hash for the chunk id type (needed to store the chunks in the unordered map
// accessed by their id)
namespace std {
    template<>
    struct hash<ChunkID> {
        size_t operator()(const ChunkID& chunk_id) const noexcept {
            return (static_cast<uint64_t>(static_cast<uint32_t>(chunk_id.x)) << 32) | static_cast<uint32_t>(chunk_id.y);
        }
    };
}

namespace voxeng {

using UserID = uint64_t;
using EntityID = uint32_t;

}
