#include "window.hpp"
#include <stdexcept>

DisplayWindow::DisplayWindow()
{
	init_window();
}

DisplayWindow::~DisplayWindow()
{
	destroy_resources();
}


void DisplayWindow::init_window()
{
	// Init window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


	this->window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "test window", nullptr, nullptr);

}

bool DisplayWindow::is_window_alive()
{
	return !glfwWindowShouldClose(this->window);
}

void DisplayWindow::destroy_resources()
{
	glfwDestroyWindow(this->window);
	//vkDestroySurfaceKHR(m_core_instance.get_instance(), m_surface, nullptr);
	glfwTerminate();
}


void DisplayWindow::init_vulkan_surface(VkInstance instance)
{
	// glfwCreateWindowSurface will do this :
	/*
	if (glfwCreateWindowSurface(instance, window, nullptr, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//The glfwGetWin32Window function is used to get the raw HWND from the GLFW window object. 
	createInfo.hwnd = glfwGetWin32Window(this->window);
	//The GetModuleHandle call returns the HINSTANCE handle of the current process.
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
	}
	*/

}
