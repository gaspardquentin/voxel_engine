#pragma once

#include <cstdint>
#include <string>
#include <vector>

/* TODO: think of the better solution
enum class VoxelID : uint8_t {
    AIR,
    ROCK,
    DIRT
};
*/

using VoxelID = uint8_t;

class VoxelType {
    VoxelID id;
    std::string name;
    bool is_solid;
    bool is_transparent;

public:
    VoxelType(VoxelID id, std::string name, bool isSolid, bool isTransparent):
        id(id), name(name), is_solid(isSolid), is_transparent(isTransparent) {}

    VoxelID getId() const { return id; }
    std::string getName() const { return name; }
    bool isTransparent() const { return is_transparent; }
    bool isSolid() const { return is_solid; }
};

const std::vector<VoxelType> DEFAULT_VOXEL_TYPES = {
    { 0, "air", false, true },
    { 1, "dirt", true, false },
};
