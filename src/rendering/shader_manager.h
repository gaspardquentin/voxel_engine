#include "rendering/shader.h"
#include <memory>
#include <string>
#include <unordered_map>

//TODO: maybe enables dynamic update of the shader
class ShaderManager {
  std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;

public:
  bool load(std::string name, std::string vertex_path, std::string fragment_path);
  std::shared_ptr<Shader> get(std::string name);
};
