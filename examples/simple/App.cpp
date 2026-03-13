#include "App.h"
#include <cstdint>

App::App(GLFWwindow *window):
    m_state(START_MENU),
    m_window(window) {}

void App::togglePause() {
    switch (m_state) {
        case START_MENU:
            return;
        case PAUSE:
            m_firstMouse = true;
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_state = GAME;
            break;
        case GAME:
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_state = PAUSE;
            break;
    }
}

void App::toggleIncreasedSpeed(voxeng::client::Client& client, uint8_t increase_factor) {
    switch (m_state) {
        case START_MENU:
        case PAUSE:
            return;
        case GAME:
            if (!m_increased_speed) {
                m_initial_speed = client.getPlayerSpeed();
                client.setPlayerSpeed(client.getPlayerSpeed() * increase_factor);
                m_increased_speed = true;
            } else {
                client.setPlayerSpeed(m_initial_speed);
                m_increased_speed = false;
            }
            break;
    }
}


void App::update(voxeng::client::Client& client) {
    if (client.isWorldOpen() && m_state == START_MENU) {
        m_firstMouse = true;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_state = GAME;
    }
    if (!client.isWorldOpen() && m_state != START_MENU) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_state = START_MENU;
    }
}

AppState App::getState() const {
    return m_state;
}

bool App::isPaused() const {
    return m_state != GAME;
}

bool App::consumeFirstMouse() {
    if (m_firstMouse) {
        m_firstMouse = false;
        return true;
    }
    return false;
}
