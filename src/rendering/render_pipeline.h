#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "rendering/interface/irender_pass.h"
#include "rendering/passes/glworld_render_pass.h"
#include "voxel_engine/world.h"
#include "camera.h"


class RenderPipeline {
  std::vector<std::unique_ptr<IRenderPass>> m_render_passes;

public:
  RenderPipeline() {}


  //TODO: maybe better doc
  // Since there is only one pass per pass kind, 
  // we can retrieve the specific pass using it's class name.
  template <typename T>
  T* getPass() {
    for (auto& pass: m_render_passes) {
      if (auto p = dynamic_cast<T*>(pass.get())) {
        return p;
      }
    }
    return nullptr;
  }

  // Adds a new pass, or retrieve the already existing
  // one (The passes instances should be unique)
  template <typename T, typename ...Args>
  T* addPass(Args&& ...args) {
    T *pass = getPass<T>();
    if (pass) {
      return pass;
    }

    auto newPass = std::make_unique<T>(std::forward<Args>(args)...);
    T *ret = newPass.get();
    m_render_passes.push_back(std::move(newPass));
    return ret;
  }

  void render(const Camera& camera);
};

