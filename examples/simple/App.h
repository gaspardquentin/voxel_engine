#pragma once

#include "voxel_engine/client/client.h"
#include <GLFW/glfw3.h>
#include <cstdint>

// App state management — handles state transitions and side effects (cursor mode, etc.)

enum AppState {
	START_MENU,
	PAUSE,
	GAME
};

class App {
	AppState m_state;
	GLFWwindow *m_window;
	bool m_firstMouse = true;
	float m_initial_speed = 0.0f;
	bool m_increased_speed = false;

public:
	App(GLFWwindow *window);

	void togglePause();
	void toggleIncreasedSpeed(voxeng::client::Client& client, uint8_t increase_factor = 10);
	void update(voxeng::client::Client& client);

	AppState getState() const;
	bool isPaused() const;
	bool consumeFirstMouse();
};
