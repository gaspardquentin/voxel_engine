#pragma once

#include "voxel_engine/chat.h"
#include "voxel_engine/world.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>

#define COMMAND_PREFIX '/'

namespace voxeng {

struct CommandContext {
    Chat& chat;
    ::World& world;
};

using CommandHandler = std::function<void(const std::vector<std::string>& args, CommandContext& ctx)>;

class CommandRegistry {
public:
    void registerCommand(const std::string& name, CommandHandler handler);
    bool tryExecute(const std::string& input, CommandContext& ctx);

private:
    std::unordered_map<std::string, CommandHandler> m_commands;
};

inline CommandHandler console_log = [](const std::vector<std::string>& args, CommandContext&) {
    std::cout << "<console_log>: ";
    for (const auto& s: args) {
        std::cout << s << " ";
    }
    std::cout << std::endl;
};

inline CommandHandler fill = [](const std::vector<std::string>& args, CommandContext& ctx) {
    if (args.size() != 3) {
        ctx.chat.sendMessage("error", "Invalid command arguments. Usage: /fill <VoxelType> <start_pos> <end_pos> \nExample: /fill air (1,1,1) (10,10,10)");
        return;
    }
    const auto& voxel_types = ctx.world.getVoxelTypes();
    auto vti = std::find_if(voxel_types.begin(), voxel_types.end(), [&args](const auto& vt) {
        return vt.getName() == args[0];
    });

    if (vti == voxel_types.end()) {
        std::string vts;
        for (const auto& type: voxel_types) {
            vts += type.getName() + "\n";
        }
        ctx.chat.sendMessage("error", "Invalid voxel type. Please pick between those : \n" + vts);
    }
    const VoxelType& vt = *vti;
    //TODO: continue this
    try {
        Vec3i v_start = Vec3i::fromString(args[1]);
        Vec3i v_end = Vec3i::fromString(args[2]);
        Vec3i lo = { 
            std::min(v_start.x, v_end.x),
            std::min(v_start.y, v_end.y),
            std::min(v_start.z,v_end.z) };
        Vec3i hi = { 
            std::max(v_start.x, v_end.x),
            std::max(v_start.y, v_end.y),
            std::max(v_start.z,v_end.z) };

        for (int x = lo.x; x <= hi.x; ++x)
            for (int y = lo.y; y <= hi.y; ++y)
                for (int z = lo.z; z <= hi.z; ++z)
                    ctx.world.setVoxel(
                        { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) }
                        ,vt.getId());
    } catch (std::invalid_argument error) {
        ctx.chat.sendMessage("error", "Invalid coordinates.");
        return;
    }
};

const std::vector<std::pair<std::string, CommandHandler>> DEFAULT_COMMANDS = {
    { "/clg", console_log },
    {"/fill", fill }
};

}
