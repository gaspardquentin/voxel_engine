#pragma once

#include "voxel_engine/server/chat.h"
#include "voxel_engine/server/world.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>

#define COMMAND_PREFIX '/'

namespace voxeng::server {

struct CommandContext {
    Chat& chat;
    World& world;
};

using CommandHandler = std::function<void(const std::vector<std::string>& args, CommandContext& ctx)>;

class CommandRegistry {
public:
    void registerCommand(const std::string& name, CommandHandler handler);
    bool tryExecute(const std::string& input, CommandContext& ctx);

private:
    std::unordered_map<std::string, CommandHandler> m_commands;
};


}
