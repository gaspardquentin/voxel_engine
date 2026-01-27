#pragma once

#include "voxel_engine/texture.h"
#include "texture_impl.h"
#include "voxel_engine/voxel_types.h"

#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

//TODO: maybe move this elsewhere
constexpr int DEFAULT_TEXTURE_WIDTH = 128;
constexpr int DEFAULT_TEXTURE_HEIGHT = 128;


class OpenGLTextureArray {
  const std::vector<VoxelType>& m_voxel_types;
  int m_texture_width = DEFAULT_TEXTURE_WIDTH;
  int m_texture_height = DEFAULT_TEXTURE_HEIGHT;
  unsigned int m_texture_array;

public:
  OpenGLTextureArray(const std::vector<VoxelType>& voxel_types): m_voxel_types(voxel_types) {}
  OpenGLTextureArray(const std::vector<VoxelType>& voxel_types, int texture_width, int texture_height):
    m_voxel_types(voxel_types), m_texture_width(texture_width), m_texture_height(texture_height) {}


  void loadTextures() {

    bool size_detected = false;
    for (const VoxelType& vt: m_voxel_types) {
      auto t = vt.getTexture();
      if (t != nullptr) {
        t->loadTexture();
        if (!size_detected && t->getImageData() != nullptr) {
            m_texture_width = t->getWidth();
            m_texture_height = t->getHeight();
            size_detected = true;
        }
      }
    }

    glGenTextures(1, &m_texture_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);

    glTexImage3D(
      GL_TEXTURE_2D_ARRAY,
      0,
      GL_RGBA8,
      m_texture_width,
      m_texture_height,
      m_voxel_types.size(), // Use full size so that ID N maps to Layer N
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr
    );

    for (const VoxelType& vt: m_voxel_types) {
      auto t = vt.getTexture();
      if (t != nullptr) {
        if (t->getImageData() == nullptr) {
          std::cerr << "<voxeng> Failed to load texture " << t->getTexturePath() << std::endl;
          continue;
        }

        glTexSubImage3D(
          GL_TEXTURE_2D_ARRAY,
          0,
          0, 0, vt.getId(),
          m_texture_width, m_texture_height, 1,
          GL_RGBA,
          GL_UNSIGNED_BYTE,
          t->getImageData()
        );
      }

    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    for (const VoxelType& vt: m_voxel_types) {
      auto t = vt.getTexture();
      if (t != nullptr) {
        t->unloadTexture();
      }
    }

  }

  void bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
  }

};
