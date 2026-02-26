#pragma once

#include "voxel_engine.h"
#include "voxel_engine/save_manager.h"
#include <string>
#include <vector>

namespace voxeng {

class GUILayer {
public:
    void drawDebugUI(const VoxelEngine& engine, float fps);
    void drawWorldsUI(VoxelEngine& engine);
    void drawChatUI(VoxelEngine& engine, std::string username);

private:
    char m_world_name_buf[128] = "my_world";
    char m_chat_input_buf[256] = {};
};

}
