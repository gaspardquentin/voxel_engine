#pragma once

#include "voxel_engine/client/texture.h"
#include "voxel_engine/voxel_types.h"

#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <unordered_map>

//TODO: maybe move this elsewhere
constexpr int DEFAULT_TEXTURE_WIDTH = 128;
constexpr int DEFAULT_TEXTURE_HEIGHT = 128;

namespace voxeng::client {

class OpenGLTextureArray {
  int m_texture_width = DEFAULT_TEXTURE_WIDTH;
  int m_texture_height = DEFAULT_TEXTURE_HEIGHT;
  unsigned int m_texture_array = 0;

public:
  void loadTextures(const std::vector<VoxelType>& voxel_types) {
    // Determine the max VoxelID to size the texture array layers
    VoxelID max_id = 0;
    for (const auto& vt : voxel_types) {
      if (vt.getId() > max_id) max_id = vt.getId();
    }

    // Load all textures and detect size from first valid one
    std::unordered_map<VoxelID, Texture> loaded;
    bool size_detected = false;
    for (const auto& vt : voxel_types) {
      if (vt.getTexture().empty()) continue;
      std::string path = std::string(VOXEL_ENGINE_ASSETS_DIR) + "/textures/blocks/" + vt.getTexture() + ".png";
      Texture tex(path);
      tex.loadTexture();
      if (!size_detected && tex.getImageData() != nullptr) {
        m_texture_width = tex.getWidth();
        m_texture_height = tex.getHeight();
        size_detected = true;
      }
      loaded.emplace(vt.getId(), std::move(tex));
    }

    glGenTextures(1, &m_texture_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);

    glTexImage3D(
      GL_TEXTURE_2D_ARRAY,
      0,
      GL_RGBA8,
      m_texture_width,
      m_texture_height,
      max_id + 1, // Layers so that ID N maps to layer N
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr
    );

    for (auto& [id, tex] : loaded) {
      if (tex.getImageData() == nullptr) {
        std::cerr << "<voxeng> Failed to load texture " << tex.getTexturePath() << std::endl;
        continue;
      }

      glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        0, 0, id,
        m_texture_width, m_texture_height, 1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        tex.getImageData()
      );
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Textures are unloaded when `loaded` goes out of scope (destructors)
  }

  void bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
  }

};

} // namespace voxeng::client
