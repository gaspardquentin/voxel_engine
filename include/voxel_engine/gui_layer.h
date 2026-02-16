#pragma once

#include "voxel_engine.h"
#include "voxel_engine/save_manager.h"

namespace voxeng {

class GUILayer {
public:
    void drawDebugUI(const VoxelEngine& engine, float fps);
    void drawSavesUI(VoxelEngine& save_manager);
};

}
