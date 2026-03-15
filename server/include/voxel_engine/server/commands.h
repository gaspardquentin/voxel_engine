#pragma once

#include "voxel_engine/server/command_registry.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/server/entity_components.h"
#include "voxel_engine/voxel_types.h"
#include <iostream>

namespace voxeng::server {

inline CommandHandler console_log = [](const std::vector<std::string>& args, CommandContext&) {
    std::cout << "<console_log>: ";
    for (const auto& s: args) {
        std::cout << s << " ";
    }
    std::cout << std::endl;
};

inline CommandHandler cow = [](const std::vector<std::string>&, CommandContext& ctx) {
    //TODO: replace this hardcoded positions with the current player pos
    auto cow = ctx.world.spawnEntity("cow", {0.0f, 15.0f, 0.0f});
    ctx.world.getRegistry().emplace<Velocity>(cow, Velocity{{10.0f, 0.0f, 0.0f}});
    ctx.log("Cow placed !!");
};


inline CommandHandler teapot = [](const std::vector<std::string>&, CommandContext& ctx) {
    //TODO: replace this hardcoded positions with the current player pos
    ctx.world.spawnEntity("teapot", {20.0f, 12.0f, 20.0f});
};

inline CommandHandler conveyor = [](const std::vector<std::string>&, CommandContext& ctx) {
    //TODO: replace this hardcoded positions with the current player pos
    ctx.world.spawnEntity("conveyor", {20.0f, 12.0f, 20.0f});
};

inline CommandHandler fill = [](const std::vector<std::string>& args, CommandContext& ctx) {
    if (args.size() != 3) {
        ctx.error("Invalid command arguments.\nUsage: /fill <VoxelType> <start_pos> <end_pos>\nExample: /fill air (1,1,1) (10,10,10)");
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
        ctx.error("Invalid voxel type. Please pick between those:\n" + vts);
        return;
    }
    const VoxelType& vt = *vti;

    try {
        Vec3i v_start = Vec3i::fromString(args[1]);
        Vec3i v_end = Vec3i::fromString(args[2]);
        Vec3i lo = {
            std::min(v_start.x, v_end.x),
            std::min(v_start.y, v_end.y),
            std::min(v_start.z, v_end.z) };
        Vec3i hi = {
            std::max(v_start.x, v_end.x),
            std::max(v_start.y, v_end.y),
            std::max(v_start.z, v_end.z) };

        for (int x = lo.x; x <= hi.x; ++x)
            for (int y = lo.y; y <= hi.y; ++y)
                for (int z = lo.z; z <= hi.z; ++z)
                    ctx.world.setVoxel(
                        { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) },
                        vt.getId());
    } catch (const std::invalid_argument&) {
        ctx.error("Invalid coordinates.");
    }
};

inline void registerDefaultCommands(CommandRegistry& registry) {
    registry.registerCommand("/clg", console_log);
    registry.registerCommand("/fill", fill);
    registry.registerCommand("/cow", cow);
    registry.registerCommand("/teapot", teapot);
    registry.registerCommand("/conveyor", conveyor);
}

}
