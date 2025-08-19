#pragma once

#include "voxel_engine/texture.h"
#include "texture_impl.h"

#include <GLFW/glfw3.h>
#include <iostream>


class OpenGLTexture: public Texture {

  unsigned int m_texture;

public:
  OpenGLTexture(const Texture& texture): Texture(texture) {}


  void loadTexture() {
    m_impl->loadTexture();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    if (m_impl->m_data != nullptr) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_impl->m_width, m_impl->m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_impl->m_data);
    } else {
      std::cerr << "<voxeng> Failed to load texture " << m_impl->m_atlas_path << std::endl;
    }

    if (m_impl->m_generate_mip_maps) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_impl->unloadTexture();
  }

  void bind() {
    glBindTexture(GL_TEXTURE_2D, m_texture);
  }

};
