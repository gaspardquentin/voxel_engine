#pragma once

#include "world.h"

enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };


struct VoxelEngineConfig {
    unsigned int width = 1280;
    unsigned int height = 720;
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

    bool playerPlaceVoxel(uint8_t max_reach, VoxelID voxel);
    bool playerRemoveVoxel(uint8_t max_reach);
    bool playerSetVoxel(uint8_t max_reach, VoxelID new_voxel); //TODO: maybe move this into voxelengine constructor arg

private:
    class Impl;
    Impl* m_impl;
};
