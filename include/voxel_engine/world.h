#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "callbacks.h"
#include "texture.h"
#include "voxel_types.h"
#include "chunk.h"
#include "math_utils.h"
#include "save_format.h"


// render distance in chunk nbrs
// TODO: maybe move this to config or even game
#define DEFAULT_RENDER_DISTANCE 16

class World {
public:
    // TODO: use those when callback system is implemented
    //void setChunkGenerator(ChunkGeneratorCallback callback);
    //void setVoxelChange(VoxelChangeCallback callback);

    // Important : the id 0 should imperatively be for AIR
    void setVoxelTypes(std::vector<VoxelType> voxel_types);

    const std::vector<VoxelType>& getVoxelTypes() const;
    const VoxelType& getVoxelType(VoxelID vid) const;

    // returns old voxel value
    VoxelID setVoxel(WorldCoord pos, VoxelID new_voxel);
    const VoxelType& getVoxel(WorldCoord pos) const;

    const std::unordered_map<ChunkID, Chunk>& getChunks() const;

    void setTextureForType(VoxelID vid, std::shared_ptr<Texture> texture);

    // Serialization / Deserialization
    static World fromData(const WorldSaveData& data);
    WorldSaveData toData() const;

    World(uint8_t render_distance, uint64_t seed);
    World(uint64_t seed);
    ~World();

    void update();
    void setRenderDistance(uint8_t render_distance); //TODO: I don't think it belongs here,
    // I guess the engine or camera should be the one asking for generation or something like that
    uint8_t getRenderDistance() const;
    void setSeed(uint64_t seed);
    uint64_t getSeed() const;

private:
    class Impl;
    Impl* m_impl;
};
