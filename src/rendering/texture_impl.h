#pragma once

#include "voxel_engine/texture.h"

using namespace voxeng;

class Texture::Impl {
public:
    std::string m_atlas_path;
    int m_tile_size;
    bool m_generate_mip_maps;

    // data to be loaded
    int m_width, m_height, m_nr_channels;
    unsigned char *m_data;
    bool m_loaded = false;

    Impl(std::string texture_path, int tile_size, bool generate_mip_maps);

    Impl(const Impl* other_impl);

    ~Impl();

    void loadTexture();

    void unloadTexture();
};
