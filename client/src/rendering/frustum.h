#pragma once

#include <array>
#include <glm/glm.hpp>

namespace voxeng::client {

// View frustum as 6 planes (ax + by + cz + d >= 0 means inside)
struct Frustum {
  std::array<glm::vec4, 6> planes;

  // Gribb-Hartmann plane extraction from a projection * view matrix
  static Frustum fromMatrix(const glm::mat4& vp) {
    // glm is column-major: vp[col][row], build rows first
    glm::vec4 row0{vp[0][0], vp[1][0], vp[2][0], vp[3][0]};
    glm::vec4 row1{vp[0][1], vp[1][1], vp[2][1], vp[3][1]};
    glm::vec4 row2{vp[0][2], vp[1][2], vp[2][2], vp[3][2]};
    glm::vec4 row3{vp[0][3], vp[1][3], vp[2][3], vp[3][3]};

    Frustum f;
    f.planes = {
      row3 + row0, // left
      row3 - row0, // right
      row3 + row1, // bottom
      row3 - row1, // top
      row3 + row2, // near
      row3 - row2, // far
    };
    for (auto& p: f.planes) {
      p /= glm::length(glm::vec3(p));
    }
    return f;
  }

  bool intersectsAABB(const glm::vec3& min, const glm::vec3& max) const {
    for (const auto& p: planes) {
      // corner of the box the farthest along the plane normal
      glm::vec3 positive{
        p.x >= 0.f ? max.x : min.x,
        p.y >= 0.f ? max.y : min.y,
        p.z >= 0.f ? max.z : min.z,
      };
      if (glm::dot(glm::vec3(p), positive) + p.w < 0.f) {
        return false;
      }
    }
    return true;
  }
};

}
