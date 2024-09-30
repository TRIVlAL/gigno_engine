#include "input_server.h"
#include "../rendering/window.h"
#include <iostream>
#include <algorithm>

namespace gigno {

	void giInputServer::BindWindow(GLFWwindow *window) {
		m_pWindow = window;
	}

	void giInputServer::UnbindWindow() {
		m_pWindow = nullptr;
	}

	void giInputServer::UpdateInput() {
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

	bool giInputServer::GetKey(Key_t key) {
		return (m_KeyStates[key] >> 1) == 1;
	}

	bool giInputServer::GetKeyDown(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_PRESSED;
	}

	bool giInputServer::GetKeyUp(Key_t key) {
		return m_KeyStates[key] == KEY_STATE_JUST_RELEASED;
	}
 
}