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
    ~Texture();
};


const std::vector<Texture> DEFAULT_TEXTURES = {
    {VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/minecraft_dirt.jpg"}
    //,{VOXEL_ENGINE_ASSETS_DIR "/textures/blocks/minecraft_grass.jpg"}
};

}
