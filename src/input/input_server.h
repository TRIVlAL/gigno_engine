#ifndef INPUT_SERVER_H
#define INPUT_SERVER_H
#include "GLFW/glfw3.h"

#include <vector>



namespace gigno {
	class giInputServer {
	public:
		giInputServer() {};
		~giInputServer() {};

		void BindWindow(GLFWwindow *window);
		void UnbindWindow(GLFWwindow *window);

		bool GetKeyDown(int keyCode);
		bool GetKeyUp(int keyCode);
		bool GetKey(int keyCode);

	private:
		std::vector<GLFWwindow *> m_BoundWindow{};
	};

}

#endif
