#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "voxel_engine/texture.h"
#include "texture_impl.h"

#include <string>
#include <iostream>

using namespace voxeng;

Texture::Texture(std::string texture_path, bool generate_mip_maps): 
    m_impl(new Texture::Impl(texture_path, DEFAULT_TILE_SIZE, generate_mip_maps)) {}

Texture::Texture(std::string texture_path, int tile_size, bool generate_mip_maps):
    m_impl(new Texture::Impl(texture_path, tile_size, generate_mip_maps)) {}

Texture::Texture(const Texture& texture):
    m_impl(new Texture::Impl(texture.m_impl))
{}

Texture::~Texture() { delete m_impl; }

void Texture::loadTexture() {
    m_impl->loadTexture();
}

void Texture::unloadTexture() {
    m_impl->unloadTexture();
}

unsigned char *Texture::getImageData() {
    return m_impl->m_data;
}

int Texture::getWidth() const {
    return m_impl->m_width;
}

int Texture::getHeight() const {
    return m_impl->m_height;
}

std::string Texture::getTexturePath() {
    return m_impl->m_atlas_path;
}


/* Texture implementation */
Texture::Impl::Impl(std::string texture_path, int tile_size, bool generate_mip_maps): 
    m_atlas_path(texture_path),
    m_tile_size(tile_size),
    m_generate_mip_maps(generate_mip_maps),
    m_data(nullptr) {}



Texture::Impl::Impl(const Impl* other_impl):
    m_atlas_path(other_impl->m_atlas_path),
    m_tile_size(other_impl->m_tile_size),
    m_generate_mip_maps(other_impl->m_generate_mip_maps),
    m_data(nullptr)
{}


Texture::Impl::~Impl() {
    if (m_loaded) {
        unloadTexture();
    }
}

void Texture::Impl::loadTexture() {
    m_data = stbi_load(m_atlas_path.c_str(), &m_width, &m_height, &m_nr_channels, 4);
    m_loaded = true;
}

void Texture::Impl::unloadTexture() {
    stbi_image_free(m_data);
    m_loaded = false;
}
