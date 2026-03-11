#pragma once

#include "voxel_engine/client/client.h"
#include <GLFW/glfw3.h>

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

public:
	App(GLFWwindow *window);

	void togglePause();
	void update(voxeng::client::Client& client);

	AppState getState() const;
	bool isPaused() const;
	bool consumeFirstMouse();
};
