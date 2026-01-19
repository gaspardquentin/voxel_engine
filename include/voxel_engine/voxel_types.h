#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "texture.h"

using namespace voxeng; //TODO: maybe encapsulate everything with the namespace

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
    std::shared_ptr<Texture> texture;

public:
    VoxelType(VoxelID id, std::string name, bool isSolid, bool isTransparent):
        id(id), name(name),
        is_solid(isSolid), is_transparent(isTransparent),
        texture(nullptr) {}

    VoxelType(VoxelID id, std::string name, bool isSolid, bool isTransparent, std::shared_ptr<Texture> texture):
        id(id), name(name),
        is_solid(isSolid), is_transparent(isTransparent),
        texture(texture) {}

    VoxelID getId() const { return id; }
    std::string getName() const { return name; }
    bool isTransparent() const { return is_transparent; }
    bool isSolid() const { return is_solid; }
    std::shared_ptr<Texture> getTexture() const { return texture; }
    void setTexture(std::shared_ptr<Texture> new_texture) { texture = new_texture; }
};

const std::vector<VoxelType> DEFAULT_VOXEL_TYPES = {
    { 0, "air", false, true, nullptr },
    { 1, "gravel", true, false, std::make_shared<Texture>(DEFAULT_TEXTURES[0]) },
    { 2, "dirt", true, false, std::make_shared<Texture>(DEFAULT_TEXTURES[1]) },
};
