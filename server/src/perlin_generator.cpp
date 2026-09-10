#include "voxel_engine/chunk.h"
#include "voxel_engine/server/chunk_generators.h"
#include "stb_perlin.h"
#include <array>
#include <cstdint>


namespace voxeng::server {

static constexpr float FREQUENCY = 0.02f;
static constexpr float LACUNARITY = 2.0f;
static constexpr float GAIN = 0.5f;
static constexpr int   OCTAVES = 4;

PerlinGenerator::PerlinGenerator(uint64_t seed): m_seed(seed) {}

void PerlinGenerator::generateHeightmap(ChunkID id, std::array<unsigned int, CHUNK_WIDTH * CHUNK_DEPTH>& heightmap, unsigned int& min_height) const {
    float offset = static_cast<float>(m_seed % 100000);
    min_height = CHUNK_HEIGHT;

    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
            float world_x = static_cast<float>(id.x * static_cast<int>(CHUNK_WIDTH) + static_cast<int>(x));
            float world_z = static_cast<float>(id.y * static_cast<int>(CHUNK_DEPTH) + static_cast<int>(z));

            float noise = stb_perlin_fbm_noise3(
                (world_x + offset) * FREQUENCY,
                (world_z + offset) * FREQUENCY,
                0.0f,
                LACUNARITY, GAIN, OCTAVES
            );

            unsigned int ground_height = static_cast<unsigned int>(
                (noise + 1.0f) * 0.5f * (CHUNK_HEIGHT - 1)
            );
            if (ground_height >= CHUNK_HEIGHT) ground_height = CHUNK_HEIGHT - 1;

            heightmap[x + z * CHUNK_WIDTH] = ground_height;
            if (ground_height < min_height) {
                min_height = ground_height;
            }
        }
    }
}

std::array<VoxelID, CHUNK_SIZE> PerlinGenerator::generate(ChunkID id) const {
    std::array<VoxelID, CHUNK_SIZE> data{};
    std::array<unsigned int, CHUNK_WIDTH * CHUNK_DEPTH> heightmap{};
    std::array<bool, CHUNK_WIDTH * CHUNK_DEPTH> is_water{};
    unsigned int min_height;

    generateHeightmap(id, heightmap, min_height);

    // find lakes TODO: maybe add more checks and randomness
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
            if (heightmap[x + z * CHUNK_WIDTH] == min_height) {
                is_water[x + z * CHUNK_WIDTH] = true;
            }
        }
    }

    // voxel placement
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
            unsigned int ground_height = heightmap[x + z * CHUNK_WIDTH];
            bool water = is_water[x + z * CHUNK_WIDTH];

            for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
                VoxelID voxel = 0; // air

                //TODO: replace the magic values of voxel types with another system
                if (y < ground_height - 2) {
                    voxel = 3; // stone
                } else if (y < ground_height) {
                    voxel = 2; // dirt
                } else if (y == ground_height) {
                    voxel = water ? 8 : 5; // 8 for water, 5 for grass
                } else if (y > ground_height && y <= min_height && water) {
                    voxel = 8;
                }

                data[Chunk::linearIndex({x, y, z})] = voxel;
            }
        }
    }

    return data;
}

}
