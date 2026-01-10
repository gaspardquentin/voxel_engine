#include "shader_manager.h"
#include <memory>

bool ShaderManager::load(std::string name, std::string vertex_path, std::string fragment_path) {
    if (m_shaders.find(name) != m_shaders.end()) {
	std::cerr << "<voxeng> : shader " << name << " already exists.\n";
	return false;
    }
    m_shaders.insert({name, std::make_shared<Shader>(vertex_path, fragment_path)});
    return true;
}

std::shared_ptr<Shader> ShaderManager::get(std::string name) {
    if (m_shaders.find(name) == m_shaders.end()) {
	std::cerr << "<voxeng> : no shader " << name << " loaded. Please load it first.\n";
	return std::shared_ptr<Shader>(nullptr);
    }
    return m_shaders.at(name);
}
