#include "input_server.h"
#include "../rendering/window.h"
#include <iostream>
#include <algorithm>

namespace gigno {

	InputServer::InputServer() {
		for(int i = 0; i < KEY_MAX_ENUM + MOUSE_BUTTON_MAX_ENUM; i++) {
			m_KeyStates[i] = KEY_STATE_RELEASED;
		}
	}

	void InputServer::BindWindow(GLFWwindow *window) {
		m_pWindow = window;
	}

	void InputServer::UnbindWindow() {
		m_pWindow = nullptr;
	}


    void InputServer::UpdateInput()
    {
        //KEYBOARD
		for(size_t i = 0; i < KEY_MAX_ENUM; i++) {
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

		// MOUSE BUTTON
		for(size_t i = KEY_MAX_ENUM; i < KEY_MAX_ENUM + MOUSE_BUTTON_MAX_ENUM; i++) {
			int new_state = glfwGetMouseButton(m_pWindow, i - KEY_MAX_ENUM);
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

		//MOUSE POSITION
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

	bool InputServer::GetKeyUp(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_RELEASED;
	}

	bool InputServer::GetKeyDown(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_PRESSED;
	}

    bool InputServer::GetMouseButton(MouseButton_t button) {
        return (m_KeyStates[KEY_MAX_ENUM + button] >> 1) == 1;
    }

    bool InputServer::GetMouseButtonUp(MouseButton_t button) {
        return m_KeyStates[KEY_MAX_ENUM + button] == KEY_STATE_JUST_RELEASED;
    }

    bool InputServer::GetMouseButtonDown(MouseButton_t button) {
		return m_KeyStates[KEY_MAX_ENUM + button] == KEY_STATE_JUST_PRESSED;
	}

    glm::vec2 InputServer::GetMousePosition() {
		return m_CurrentMousePos;
    }

    glm::vec2 InputServer::GetMouseDelta() {
        glm::vec2 delta = m_CurrentMousePos - m_LastMousePos;

		//NaN prevention
		delta.x = delta.x == delta.x ? delta.x : 0.0f;
		delta.y = delta.y == delta.y ? delta.y : 0.0f;
		
		return delta;
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
 
}