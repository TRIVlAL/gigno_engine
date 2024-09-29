#include "input_server.h"
#include "../rendering/window.h"
#include <iostream>
#include <algorithm>

namespace gigno {

	void giInputServer::BindWindow(GLFWwindow *window) {
		m_BoundWindow.push_back(window);
	}

	void giInputServer::UnbindWindow(GLFWwindow *window) {
		m_BoundWindow.erase(std::remove(m_BoundWindow.begin(), m_BoundWindow.end(), window), m_BoundWindow.end());
	}

	bool giInputServer::GetKeyUp(int keyCode) {
		for (GLFWwindow *window : m_BoundWindow) {
			int state = glfwGetKey(window, keyCode);
			if (state == GLFW_RELEASE) {
				return true;
			}
		}
		return false;
	}

	bool giInputServer::GetKey(int keyCode) {
		for (GLFWwindow *window : m_BoundWindow) {
			int state = glfwGetKey(window, keyCode);
			if (state == GLFW_PRESS || state == GLFW_REPEAT) {
				return true;
			}
		}
		return false;
	}
 
}