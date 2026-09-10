#include "voxel_engine/server/chunk_generators.h"

namespace voxeng::server {

std::array<VoxelID, CHUNK_SIZE> FlatGenerator::generate(ChunkID id) const {
    (void)id;
    std::array<VoxelID, CHUNK_SIZE> data{};
    unsigned int ground_height = CHUNK_HEIGHT / 2;
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
            for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
                //TODO: replace the magic values of voxel types with another system
                VoxelID voxel = 0; // Air
                if (y < ground_height - 2) {
                    voxel = 3; // Stone
                } else if (y < ground_height) {
                    voxel = 2; // Dirt
                } else if (y == ground_height) {
                    voxel = 5; // Grass
                }
                data[Chunk::linearIndex({x, y, z})] = voxel;
            }
        }
    }
    return data;
}

}
