#pragma once

#include "voxel_engine/client/texture.h"
#include "texture_impl.h"

#include <GLFW/glfw3.h>
#include <iostream>


namespace voxeng::client {

class OpenGLTexture: public Texture {

  unsigned int m_texture;

public:
  OpenGLTexture(const Texture& texture): Texture(texture) {}


  void loadTexture() {
    m_impl->loadTexture();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (m_impl->m_data != nullptr) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_impl->m_width, m_impl->m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_impl->m_data);
    } else {
      std::cerr << "<voxeng> Failed to load texture " << m_impl->m_atlas_path << std::endl;
    }

    if (m_impl->m_generate_mip_maps) {
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_impl->unloadTexture();
  }

  void bind() {
    glBindTexture(GL_TEXTURE_2D, m_texture);
  }

};

} // namespace voxeng::client
