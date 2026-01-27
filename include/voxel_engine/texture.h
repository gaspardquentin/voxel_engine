#pragma once

#include <string>
#include <vector>


#define DEFAULT_TILE_SIZE 16

namespace voxeng {

class Texture {
    class Impl;
protected:
    Impl *m_impl;
public:
    Texture(std::string texture_path, bool generate_mip_maps = true);
    Texture(std::string texture_path, int tile_size, bool generate_mip_maps = true);
    Texture(const Texture& texture);

    void loadTexture();
    void unloadTexture();
    unsigned char *getImageData();
    int getWidth() const;
    int getHeight() const;
    std::string getTexturePath();



    ~Texture();
};


const std::vector<Texture> DEFAULT_TEXTURES = {
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/gravel_stone.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/dirt.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/stone.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/sand.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/grass_top.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/trunk_side.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/leaves.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/water.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/brick_red.png"},
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/glass.png"}
};

}
