#include "window.h"
#include "../error_macros.h"
#include "../input/input_server.h"

namespace gigno {

	giWindow::giWindow(int w, int h, const char *pTitle, giInputServer *inputServer) : m_Width(w), m_Height(h) {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_Width, m_Height, pTitle, nullptr, nullptr);

		m_UserPointerData.giwindow = this;

		glfwSetWindowUserPointer(m_pWindow, &m_UserPointerData);

		glfwSetFramebufferSizeCallback(m_pWindow, ResizeCallback);

		inputServer->BindWindow(m_pWindow);
		m_InputServerBoundTo = inputServer;
	}

	giWindow::~giWindow() {
		if (m_InputServerBoundTo) {
			m_InputServerBoundTo->UnbindWindow(m_pWindow);
		}
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	bool giWindow::ShouldClose() {
		return glfwWindowShouldClose(m_pWindow);
	}

	void giWindow::PollEvents() {
		glfwPollEvents();
	}

	void giWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const {
		VkResult result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Window Surface ! Vullkan error code : " << (int)result);
		}
	}

	void giWindow::GetFrameBufferSize(int *width, int *height) const {
		glfwGetFramebufferSize(m_pWindow, width, height);
	}

	bool giWindow::HasResized() {
		bool ret = resized;
		resized = false;
		return ret;
	}

	void giWindow::ResizeCallback(GLFWwindow *window, int width, int height) {
		WindowUserPointerData_t *data = reinterpret_cast<WindowUserPointerData_t *>(glfwGetWindowUserPointer(window));
		giWindow *giwindow = data->giwindow;
		giwindow->resized = true;
		giwindow->m_Width = width;
		giwindow->m_Height = height;
	}
}