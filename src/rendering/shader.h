#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <GL/glew.h>

class Shader {
public:
  Shader(const std::string& vertex_path, const std::string& fragment_path): 
    m_program(glCreateProgram())
  {
    std::string vertex = get_shader(vertex_path);
    std::string fragment = get_shader(fragment_path);

    if (vertex.empty() || fragment.empty()) {
      std::cerr << "Shader.hpp: Failed to load shaders" << std::endl;
      return;
    }
    const char *vertex_src = vertex.c_str();
    const char *frag_src = fragment.c_str();

    unsigned int vertex_shader = bind_shader(vertex_src, GL_VERTEX_SHADER);
    unsigned int fragment_shader = bind_shader(frag_src, GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
      std::cerr << "Shader.hpp: Shader compilation failed." << std::endl;
      exit(0);
      return;
    }

    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glLinkProgram(m_program);
  }

  //ensure no copy is possible
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  ~Shader() {
    glDeleteProgram(m_program);
  }

  bool validateProgram() {
    glValidateProgram(m_program);
    GLint success;
    glGetProgramiv(m_program, GL_VALIDATE_STATUS, &success);
    if (!success) {
      GLchar infoLog[512];
      glGetProgramInfoLog(m_program, 512, NULL, infoLog);
      std::cerr << "ERROR::SHADER::PROGRAM_VALIDATION_FAILED\n" << infoLog << std::endl;
    }
    return success;
  }


  // Methods
  void bind() const {
    glUseProgram(m_program);
  }
  void unbind() const {
    glUseProgram(0);
  }

  void setUniformMatrix4fv(const std::string& name, const float *matrix) const {
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1) {
        std::cerr << "WARNING::SHADER::UNIFORM '" << name << "' NOT FOUND" << std::endl;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
  }

  void setUniformVec3(const std::string& name, const float *vector) const {
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1) {
        std::cerr << "WARNING::SHADER::UNIFORM '" << name << "' NOT FOUND" << std::endl;
    }
    glUniform3fv(location, 1, vector);
  }


  void setUniform1i(const std::string& name, int val) const {
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1) {
      std::cerr << "WARNING::SHADER::UNIFORM '" << name << "' NOT FOUND" << std::endl;
    }
    glUniform1i(location, val);
  }

private:

  unsigned int m_program;


  unsigned int bind_shader(const char *shader_src, GLenum kind) {
    unsigned int shader = glCreateShader(kind);
    glShaderSource(shader, 1, &shader_src, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
      return 0;
    }

    return shader;
  }

  const std::string get_shader(const std::string filepath) {
    std::ifstream stream(filepath);
    if (stream) {
      std::string s((std::istreambuf_iterator<char>(stream)),
                    std::istreambuf_iterator<char>());
      return s;
    }
    return "";
  }
};
