#pragma once

#include <string>


#define DEFAULT_TILE_SIZE 16

namespace voxeng::client {

class Texture {
    class Impl;
protected:
    Impl *m_impl;
public:
    Texture(std::string texture_path, bool generate_mip_maps = true);
    Texture(std::string texture_path, int tile_size, bool generate_mip_maps = true);
    Texture(const Texture& texture);
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void loadTexture();
    void unloadTexture();
    unsigned char *getImageData();
    int getWidth() const;
    int getHeight() const;
    std::string getTexturePath();



    ~Texture();
};


}
