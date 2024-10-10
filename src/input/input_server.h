#ifndef INPUT_SERVER_H
#define INPUT_SERVER_H
#include "GLFW/glfw3.h"
#include "keys.h"

#include <vector>



namespace gigno {
	enum KeyState_t {
		// Order Matters !
		KEY_STATE_RELEASED = 0b00,
		KEY_STATE_JUST_RELEASED = 0b01,
		KEY_STATE_PRESSED = 0b10,
		KEY_STATE_JUST_PRESSED = 0b11
	};

	class InputServer {
	public:
		InputServer() {};
		~InputServer() {};

		void UpdateInput();

		void BindWindow(GLFWwindow *window);
		void UnbindWindow();

		bool GetKey(Key_t key);
		bool GetKeyUp(Key_t key);
		bool GetKeyDown(Key_t key);

	private:
		GLFWwindow* m_pWindow;
		KeyState_t m_KeyStates[KEY_MAX_ENUM];
	};

}

#endif
