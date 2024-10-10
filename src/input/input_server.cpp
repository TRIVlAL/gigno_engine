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
	}

	bool InputServer::GetKey(Key_t key) {
		return (m_KeyStates[key] >> 1) == 1;
	}

	bool InputServer::GetKeyDown(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_PRESSED;
	}

	bool InputServer::GetKeyUp(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_RELEASED;
	}
 
}