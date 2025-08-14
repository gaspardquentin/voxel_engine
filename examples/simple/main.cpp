#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "voxel_engine/voxel_engine.h"

int main() {
    std::cout << "Basic demo of the voxel engine." << std::endl;

    /* ====== opengl context init ======= */
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Cube", NULL, NULL);
    if (!window) {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    /*
    glDebugMessageCallback(
        [](GLenum src, GLenum type, GLuint id, GLenum sev, GLsizei len,
           const GLchar *msg, const void *user)->void {
            std::cerr << "GL DEBUG: " << msg << " (sev="<<sev<<", id="<<id<<")\n";
        }, nullptr);
        */
    std::cout << "here" << std::endl;

    /* ====== engine demo ======= */
    VoxelEngine engine; //TODO: maybe separate engine from renderer
    while (!glfwWindowShouldClose(window)) {
        engine.render(); //TODO: change this
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Demo end. Goodbye !" << std::endl;
    return EXIT_SUCCESS;
}
