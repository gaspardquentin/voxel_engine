#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "imgui.h"
#include "voxel_engine/voxel_engine.h"

const unsigned int SCREEN_WIDTH = 3840; // or 1920 for FHD screen
const unsigned int SCREEN_HEIGHT = 2160; // or 1080

float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;

VoxelEngine* gVoxelEngine = nullptr; //global pointer needed for callbacks

bool firstMouse = true; //TODO: maybe find better approach

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (gVoxelEngine != nullptr) {
        gVoxelEngine->processMovementCamera(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (gVoxelEngine != nullptr) {
        gVoxelEngine->processZoomCamera(static_cast<float>(yoffset));
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (gVoxelEngine == nullptr) {
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        gVoxelEngine->playerRemoveVoxel(3);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        gVoxelEngine->playerPlaceVoxel(3, 1);
    }
}


void processInput(GLFWwindow *window, VoxelEngine& engine, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        engine.processMovementPlayer(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        engine.processMovementPlayer(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        engine.processMovementPlayer(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        engine.processMovementPlayer(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        engine.processMovementPlayer(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        engine.processMovementPlayer(DOWN, deltaTime);
}


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

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel Engine example", NULL, NULL);
    if (!window) {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // set mouse callbacks for camera movements and block placement
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSwapInterval(0); // Disable V-Sync


    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    /* ====== Imgui ui setup ======= */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Automatic scaling for high-DPI/4K displays
    if (SCREEN_WIDTH > 2000) {
        float scale = 2.0f;
        ImGui::GetStyle().ScaleAllSizes(scale);
        io.FontGlobalScale = scale;
    }
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 410";
    ImGui_ImplOpenGL3_Init(glsl_version);


    /* ====== engine demo ======= */
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    VoxelEngineConfig engine_conf{
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    };
    VoxelEngine engine{engine_conf}; //TODO: maybe separate engine from renderer
    gVoxelEngine = &engine;

    /* ===== fps calculation ====== */
    double lastTime = glfwGetTime();
    int frameCount = 0;
    float currentFps = 0.0f;
    float treshold = 1.0f;
    while (!glfwWindowShouldClose(window)) {
        /* ====== Imgui ui ======= */
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float currentFrame = static_cast<float>(glfwGetTime());
        float elapsedTime = currentFrame - lastTime;
        if (elapsedTime > treshold) {
            lastTime = currentFrame;
            currentFps = frameCount / elapsedTime;
            frameCount = 0;
        }
        frameCount++;

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, engine, deltaTime);

        engine.drawDebugUI(currentFps);
        engine.render(); //TODO: change this
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Demo end. Goodbye !" << std::endl;
    return EXIT_SUCCESS;
}
