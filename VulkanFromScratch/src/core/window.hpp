#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
class DisplayWindow {
public:
	DisplayWindow();
	~DisplayWindow();
	const unsigned int SCR_WIDTH = 480;
	const unsigned int SCR_HEIGHT = 480;
	void init_window();
	bool is_window_alive();
	void init_vulkan_surface(VkInstance instance);

	void destroy_resources();
	

	//------------------
	//   Get / Set
	//------------------
	GLFWwindow* get_window() { return window; }
	

private:
	GLFWwindow* window;	
	//VkSurfaceKHR m_surface;
};
