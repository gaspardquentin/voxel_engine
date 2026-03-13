#pragma once

#include <string>
#include <vector>

#include "types.h"

namespace voxeng {

struct VoxelTypeDef {
    std::string name;
    std::string texture;
    bool solid = true;
    bool transparent = false;
};

class VoxelType {
    VoxelID m_id;
    VoxelTypeDef m_def;

public:
    VoxelType(VoxelID id, VoxelTypeDef def)
        : m_id(id), m_def(std::move(def)) {}

    VoxelID getId() const { return m_id; }
    const std::string& getName() const { return m_def.name; }
    const std::string& getTexture() const { return m_def.texture; }
    bool isSolid() const { return m_def.solid; }
    bool isTransparent() const { return m_def.transparent; }
};

const std::vector<VoxelType> DEFAULT_VOXEL_TYPES = {
    {0, {"air", "", false, true}},
    {1, {"gravel", "gravel_stone"}},
    {2, {"dirt", "dirt"}},
    {3, {"stone", "stone"}},
    {4, {"sand", "sand"}},
    {5, {"grass", "grass_top"}},
    {6, {"wood", "wood"}},
    {7, {"leaves", "leaves"}},
    {8, {"water", "water", false, true}},
    {9, {"brick", "brick_red"}},
    {10, {"glass", "glass", true, true}},
};

}
