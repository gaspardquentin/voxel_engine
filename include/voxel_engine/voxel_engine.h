#pragma once

#include "voxel_engine/voxel_types.h"
#include "world.h"

enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

enum class TextureMode { TEXTURE_ARRAY_2D, TEXTURE_ATLAS };


struct VoxelEngineConfig {
    unsigned int width = 1280;
    unsigned int height = 720;
    TextureMode texture_mode = TextureMode::TEXTURE_ARRAY_2D;
    std::string world_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/vertex.vert";
    std::string world_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/fragment.frag";
    std::string ui_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.vert"; //TODO: maybe rename shader or path
    std::string ui_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.frag";
};

class VoxelEngine {
public:
    VoxelEngine(const VoxelEngineConfig& config); //TODO: implement this
    ~VoxelEngine();

    World& getWorld();
    void render();
    void processMovementPlayer(Movement mov, float delta_time);
    void processMovementCamera(float xoffset, float yoffset, bool constrain_pitch = true);
    void processZoomCamera(float yoffset);

    //TODO: maybe remove the player from the name of those methods
    bool playerPlaceVoxel(uint8_t max_reach, VoxelID voxel);
    bool playerRemoveVoxel(uint8_t max_reach);
    bool playerSetVoxel(uint8_t max_reach, VoxelID new_voxel); //TODO: maybe move this into voxelengine constructor arg

    // UI
    void drawDebugUI(float fps) const;
    //TODO: maybe add render distance setter


private:
    class Impl;
    Impl* m_impl;
};
