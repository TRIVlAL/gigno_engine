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

	enum MouseMode_t {
		MOUSE_DEFAULT = GLFW_CURSOR_NORMAL,
		MOUSE_HIDDEN = GLFW_CURSOR_HIDDEN,
		MOUSE_LOCKED_CENTER = GLFW_CURSOR_CAPTURED,
		MOUSE_MOVE_INFINITE = GLFW_CURSOR_DISABLED	// Cursor invisible and movement unconstrained
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

		/*
		returns the pixel coordinate of the mouse relative to the upper left corner. 
		*/
		glm::vec2 GetMousePosition();
		/*
		returns the horitontal and vertical number of pixels mouved by the mouse since the last input update. 
		*/
		glm::vec2 GetMouseDelta();
		void SetMouseMode(MouseMode_t mode);
		/*
		returns the current Mouse Mode set, meaning MOUSE_DEFAULT if in mouse reaccessing mode.
		*/
		MouseMode_t GetMouseMode();

		/*
		Reaccess alows to give temporarly back the control of the mouse (to access 
		the console, for example) without breaking the current state required by the app 

		If reaccess is true Mouse Mode is forced to MOUSE_DEFAULT.
		If reaccess is false, Mouse Mode is the last mouse state set by the user.
		*/
		void SetReaccessMouse(bool reaccess);
		bool IsMouseReaccessed() { return m_IsMouseInReaccessMode; }

	private:
		// When this key is pressed, ToggleREaccessMouse is called.
		const Key_t REACCESS_MOUSE_KEY = KEY_ESCAPE;
		// Is the mouse in it's user set state, or forced to MOUSE_DEFAULT ?
		bool m_IsMouseInReaccessMode = false;
		// The Last Mouse Mode Set by user
		MouseMode_t m_MouseModeWithoutReaccess = MOUSE_DEFAULT;

		GLFWwindow* m_pWindow;
		KeyState_t m_KeyStates[KEY_MAX_ENUM];

		glm::vec2 m_LastMousePos;
		glm::vec2 m_CurrentMousePos;
	};

}

#endif
