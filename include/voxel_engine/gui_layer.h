#pragma once

#include "voxel_engine.h"
#include "voxel_engine/save_manager.h"

namespace voxeng {

class GUILayer {
public:
    void drawDebugUI(const VoxelEngine& engine, float fps);
    void drawWorldsUI(VoxelEngine& engine);

private:
    char m_world_name_buf[128] = "my_world";
};

}
