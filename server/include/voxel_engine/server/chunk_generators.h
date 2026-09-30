#pragma once

#include <array>
#include <cstdint>
#include "voxel_engine/types.h"
#include "voxel_engine/chunk.h"

namespace voxeng::server {

using RawChunk = std::array<VoxelID, CHUNK_SIZE>;

class IChunkGenerator {
public:
    virtual ~IChunkGenerator() = default;
    virtual RawChunk generate(ChunkID id) const = 0;
};

class PerlinGenerator : public IChunkGenerator {
public:
    PerlinGenerator(uint64_t seed);
    RawChunk generate(ChunkID id) const;
private:
    uint64_t m_seed;
    void generateHeightmap(ChunkID id, std::array<unsigned int, CHUNK_WIDTH * CHUNK_DEPTH>& heightmap, unsigned int& min_height) const;
};

class FlatGenerator: public IChunkGenerator {
public:
    RawChunk generate(ChunkID id) const;
};

}
