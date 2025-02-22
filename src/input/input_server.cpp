#include "input_server.h"
#include "../rendering/window.h"
#include <iostream>
#include <algorithm>

namespace gigno {

	void InputServer::BindWindow(GLFWwindow *window) {
		m_pWindow = window;
	}

	void InputServer::UnbindWindow() {
		m_pWindow = nullptr;
	}

	void InputServer::UpdateInput() {
		//KEYBOARD
		for(int i = 0; i < KEY_MAX_ENUM; i++) {
			int new_state = glfwGetKey(m_pWindow, i);
			if(new_state == GLFW_PRESS) {
				KeyState_t& state = m_KeyStates[i];
				if(state == KEY_STATE_JUST_PRESSED) {
					state = KEY_STATE_PRESSED;
				} else if(state != KEY_STATE_PRESSED) {
					state = KEY_STATE_JUST_PRESSED;
				}
			} else { //state == GLFW_RELEASED
				KeyState_t& state = m_KeyStates[i];
				if(state == KEY_STATE_JUST_RELEASED) {
					state = KEY_STATE_RELEASED;
				} else if(state != KEY_STATE_RELEASED) {
					state = KEY_STATE_JUST_RELEASED;
				}
			}
		}

		//MOUSE
		m_LastMousePos = m_CurrentMousePos;
		double x, y;
		glfwGetCursorPos(m_pWindow, &x, &y);
		m_CurrentMousePos = glm::vec2{static_cast<float>(x), static_cast<float>(y)};

		//REACCESS MOUSE
		if(GetKeyDown(REACCESS_MOUSE_KEY)) {
			SetReaccessMouse(!m_IsMouseInReaccessMode);
		}
	}

	bool InputServer::GetKey(Key_t key) {
		return (m_KeyStates[key] >> 1) == 1;
	}

	bool InputServer::GetKeyDown(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_PRESSED;
	}

    glm::vec2 InputServer::GetMousePosition() {
		return m_CurrentMousePos;
    }

    glm::vec2 InputServer::GetMouseDelta() {
        return m_CurrentMousePos - m_LastMousePos;
    }

    void InputServer::SetMouseMode(MouseMode_t mode) {
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, mode);
		m_MouseModeWithoutReaccess = mode;
    }

    MouseMode_t InputServer::GetMouseMode() {
        return (MouseMode_t)glfwGetInputMode(m_pWindow, GLFW_CURSOR);
    }

    void InputServer::SetReaccessMouse(bool reaccess) {
		if(reaccess && !m_IsMouseInReaccessMode) {
			MouseMode_t without_reaccess = GetMouseMode();
			SetMouseMode(MOUSE_DEFAULT);
			m_MouseModeWithoutReaccess = without_reaccess;
			m_IsMouseInReaccessMode = true;
		} else if(!reaccess && m_IsMouseInReaccessMode) {
			SetMouseMode(m_MouseModeWithoutReaccess);
			m_IsMouseInReaccessMode = false;
		}
    }

    bool InputServer::GetKeyUp(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_RELEASED;
	}
 
}