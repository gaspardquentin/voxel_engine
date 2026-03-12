#pragma once

#include "callbacks.h"
#include "chunk.h"

#include <cmath>

namespace voxeng {

inline ChunkID getChunkId(WorldCoord pos) {
    return {
        static_cast<int32_t>(std::floor(pos.x / static_cast<float>(Chunk::WIDTH))),
        static_cast<int32_t>(std::floor(pos.z / static_cast<float>(Chunk::DEPTH)))
    };
}

inline Vec3f getChunkWorldPos(ChunkID chunk_id) {
    return {
        static_cast<float>(chunk_id.x) * Chunk::WIDTH,
        0.0f,
        static_cast<float>(chunk_id.y) * Chunk::DEPTH
    };
}

} // namespace voxeng
