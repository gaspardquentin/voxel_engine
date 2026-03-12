#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "App.h"
#include "imgui.h"
#include "voxel_engine/client/camera.h"
#include "voxel_engine/client/client.h"
#include "voxel_engine/client/gui_layer.h"
#include "voxel_engine/client/network/local_client_connection.h"
#include "voxel_engine/network/local_transport.h"
#include "voxel_engine/server/network/local_server_connection.h"
#include "voxel_engine/server/server.h"

const unsigned int SCREEN_WIDTH = 3840; // or 1920 for FHD screen
const unsigned int SCREEN_HEIGHT = 2160; // or 1080

const std::string username = "local";

float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;

voxeng::client::Client *gClient = nullptr; //global ponter needed for callbacks
App *gApp = nullptr;


void mouse_callback(GLFWwindow*, double xposIn, double yposIn) {
    if (gApp == nullptr || gApp->isPaused()) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (gApp->consumeFirstMouse()) {
        lastX = xpos;
        lastY = ypos;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (gClient != nullptr) {
        gClient->moveCamera(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    if (gClient != nullptr) {
        gClient->zoomCamera(static_cast<float>(yoffset));
    }
}

//TODO: move those
int voxelId = 1;
std::string voxelName = "gravel";
int maxVoxel = 10; //TODO: maybe get this from a getter
int maxReach = 3;


void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (gClient == nullptr) {
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        gClient->removeVoxel(maxReach);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        gClient->placeVoxel(maxReach, voxelId);
    }
}


void processInput(GLFWwindow *window, App& app, voxeng::client::Client& client, float deltaTime) {
    static bool eWasDown = false;
    static bool escapeWasDown = false;

    if (!app.isPaused()) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            client.movePlayer(voxeng::client::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            client.movePlayer(voxeng::client::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            client.movePlayer(voxeng::client::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            client.movePlayer(voxeng::client::RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            client.movePlayer(voxeng::client::UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            client.movePlayer(voxeng::client::DOWN, deltaTime);
    }

    // Escape key event (pause mode)
    bool escapeIsDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escapeIsDown && !escapeWasDown) {
        app.togglePause();
    }
    escapeWasDown = escapeIsDown;

    // E key even (switch bloc type)
    bool eIsDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
    if (eIsDown && !eWasDown) {
        voxelId++;
        if (voxelId > maxVoxel) {
            voxelId = 1;
        }
        if (client.isWorldOpen()) {
            auto vt = client.getWorldVoxelType(voxelId);
            if (vt != nullptr) {
                voxelName = vt->getName();
            }
        }
    }
    eWasDown = eIsDown;
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

    // start with cursor visible for the world menu
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSwapInterval(0); // Disable V-Sync


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
    voxeng::client::GUILayer engine_gui;


    /* ====== engine demo ======= */
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    voxeng::client::ClientConfig client_conf;
    client_conf.width = SCREEN_WIDTH;
    client_conf.height = SCREEN_HEIGHT;

    //TODO: maybe move this into a Network builder or something (on the library itself, out of the example)
    voxeng::network::LocalTransport local_transport; // Note: see this as the link between the server and the client
    voxeng::network::LocalServerConnection server_connection{local_transport};
    voxeng::network::LocalClientConnection client_connection{local_transport};

    voxeng::client::Client client{client_conf, client_connection}; //TODO: maybe separate engine from renderer
    client.setUsername("you");
    voxeng::server::Server server{server_connection};
    App app{window};
    gClient = &client;
    gApp = &app;

    /*
    // Multithreaded server approach (commented out due to lack of performance improvement)
    std::atomic<bool> server_running{true};
    std::thread server_thread([&server, &server_running]() {
        auto last_time = std::chrono::steady_clock::now();
        while (server_running) {
            auto current_time = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = current_time - last_time;
            last_time = current_time;

            server.update(elapsed.count());

            // Yield CPU to prevent 100% core usage; Server::update handles the actual 20Hz pacing.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    */

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

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10, 10), ImGuiCond_Appearing, ImVec2(1.0f, 0.0f));
        ImGui::Begin("Game informations", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Quit Game")) {
            glfwSetWindowShouldClose(window, true);
        }
        ImGui::Text("Selected voxel: %s ; press E to change", voxelName.c_str());
        ImGui::End();

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
        processInput(window, app, client, deltaTime);
        app.update(client);

        switch (app.getState()) {
            case START_MENU:
                engine_gui.drawStartMenuUI(client);
                break;
            case GAME:
            case PAUSE:
                engine_gui.drawDebugUI(client, currentFps);
                engine_gui.drawChatUI(client);
                break;
        }

        client.update();
        server.update(deltaTime);

        client.render();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /*
    // Multithreaded server shutdown
    server_running = false;
    server_thread.join();
    */

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Demo end. Goodbye !" << std::endl;
    return EXIT_SUCCESS;
}
