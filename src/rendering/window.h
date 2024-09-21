#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include <string>

namespace gigno {
	class giInputServer;
	class giWindow;

	struct WindowUserPointerData_t {
		giWindow *giwindow;
	};

	class giWindow {

	public:
		giWindow(int w, int h, const char *pTitle, giInputServer *inputServer);
		~giWindow();

		giWindow(const giWindow &) = delete;
		giWindow& operator=(const giWindow &) = delete;

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

		giInputServer *m_InputServerBoundTo;
	};

}

#endif
