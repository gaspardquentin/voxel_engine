#pragma once

#include "voxel_engine/save_manager.h"
#include "voxel_engine/voxel_types.h"
#include "world.h"
#include "camera.h"

enum class TextureMode { TEXTURE_ARRAY_2D, TEXTURE_ATLAS };


struct VoxelEngineConfig {
    unsigned int width = 1280;
    unsigned int height = 720;
    TextureMode texture_mode = TextureMode::TEXTURE_ARRAY_2D;
    std::string world_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/vertex.vert";
    std::string world_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/fragment.frag";
    std::string ui_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.vert";
    std::string ui_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.frag";
};

class VoxelEngine {
public:
    VoxelEngine(const VoxelEngineConfig& config);
    ~VoxelEngine();

    World& getWorld();
    void setWorld(World&& world);
    Camera& getCamera();
    voxeng::SaveManager& getSaveManager();
    const Camera& getCamera() const;

    void createNewWorld(const std::string& name);
    void loadExistingWorld(const std::string& world_path);

    void render();
    void update(float delta_time);
    void processMovementPlayer(Movement mov, float delta_time);
    void processMovementCamera(float xoffset, float yoffset, bool constrain_pitch = true);
    void processZoomCamera(float yoffset);

    bool playerPlaceVoxel(uint8_t max_reach, VoxelID voxel);
    bool playerRemoveVoxel(uint8_t max_reach);
    bool playerSetVoxel(uint8_t max_reach, VoxelID new_voxel);

    // UI
    void drawDebugUI(float fps) const;

private:
    class Impl;
    Impl* m_impl;
};
