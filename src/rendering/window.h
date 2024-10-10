#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include <string>

namespace gigno {
	class InputServer;
	class Window;

	struct WindowUserPointerData_t {
		Window *Window;
	};

	class Window {

	public:
		Window(int w, int h, const char *pTitle, InputServer *inputServer);
		~Window();

		Window(const Window &) = delete;
		Window& operator=(const Window &) = delete;

		bool ShouldClose();
		void PollEvents();

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const;

		void GetFrameBufferSize(int *width, int *height) const;

		GLFWwindow *GetGLFWwindow() const  { return m_pWindow; };

		bool HasResized();

	private:
		static void ResizeCallback(GLFWwindow *window, int width, int height);

		int m_Width;
		int m_Height;

		bool resized = false;

		GLFWwindow *m_pWindow;

		WindowUserPointerData_t m_UserPointerData;

		InputServer *m_InputServerBoundTo;
	};

}

#endif
