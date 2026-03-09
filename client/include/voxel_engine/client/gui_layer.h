#pragma once

#include "voxel_engine/client/client.h"
#include <string>

namespace voxeng::client {

class GUILayer {
public:
    void drawDebugUI(const Client& client, float fps);
    void drawStartMenuUI(Client& client);
    void drawChatUI(Client& client, std::string username);

private:
    char m_world_name_buf[128] = "my_world";
    char m_chat_input_buf[256] = {};
};

}
