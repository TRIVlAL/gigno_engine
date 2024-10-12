#include "window.h"
#include "../error_macros.h"
#include "../input/input_server.h"
#include "../application.h"

namespace gigno {

	Window::Window(int w, int h, const char *pTitle, InputServer *inputServer) : m_Width(w), m_Height(h) {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_Width, m_Height, pTitle, nullptr, nullptr);

		m_UserPointerData.Window = this;

		glfwSetWindowUserPointer(m_pWindow, &m_UserPointerData);

		glfwSetFramebufferSizeCallback(m_pWindow, ResizeCallback);

		inputServer->BindWindow(m_pWindow);
		m_InputServerBoundTo = inputServer;
	}

	Window::~Window() {
		if (m_InputServerBoundTo) {
			m_InputServerBoundTo->UnbindWindow();
		}
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	bool Window::ShouldClose() {
		return glfwWindowShouldClose(m_pWindow);
	}

	void Window::PollEvents() {
		glfwPollEvents();
	}

	void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const {
		VkResult result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Window Surface ! Vullkan error code : %d", (int)result);
		}
	}

	void Window::GetFrameBufferSize(int *width, int *height) const {
		glfwGetFramebufferSize(m_pWindow, width, height);
	}

	bool Window::HasResized() {
		bool ret = resized;
		resized = false;
		return ret;
	}

	void Window::ResizeCallback(GLFWwindow *window, int width, int height) {
		WindowUserPointerData_t *data = reinterpret_cast<WindowUserPointerData_t *>(glfwGetWindowUserPointer(window));
		Window *Window = data->Window;
		Window->resized = true;
		Window->m_Width = width;
		Window->m_Height = height;
	}
}