#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "./core/window.hpp"
#include <optional>  // std:c++17 up

//#include <iostream>
class CoreInstance {
public:
	CoreInstance(GLFWwindow& window);
	~CoreInstance();

	void create_instance();
	//--------------------
	// Logical device:
	//--------------------	
	void create_device_and_queuefamily();
	void create_surface(GLFWwindow& window);

	//--------------------
	// Physical device:
	//--------------------
	void setup_physical_device();
	void setup_physical_property();
	void setup_physical_features();
	void setup_queuefamily_properties();
	bool is_physical_device_suitable(VkPhysicalDevice device);
	void cleanup();

	//--------------------
	//  Get / set
	//--------------------
	VkInstance get_instance() { return m_instance; }
	VkSurfaceKHR get_surface(){ return m_surface; }
	VkDevice get_device() { return m_device; }
	VkPhysicalDevice get_physical_device() { return m_physicalDevice; }
	inline const auto get_queuefailmy_indexs () const { return &m_queueFamilyIndex; }
	inline const VkQueue graphic_queue() const { return m_graphicsQueue;  }
	inline const VkQueue present_queue() const { return m_presentQueue; }
	inline const VkCommandPool& cmd_pool() const { return m_commandPool; }
private:
	struct QueueFamilyIndex
	{
		std::optional<uint32_t>                 graphic_queuefamily_index; // graphic command process queue family
		std::optional<uint32_t>                 present_queuefamily_index; // present surface queue family
		bool is_complete() {
			return
				graphic_queuefamily_index.has_value() && present_queuefamily_index.has_value();
		}
	}m_queueFamilyIndex;
	//--------------------
	// Logical device:
	//--------------------

	// Instance level
	VkInstance m_instance{};
	VkSurfaceKHR m_surface{};
	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> query_instance_extensions();

	// Device level
	VkDevice m_device{};
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	std::vector<const char*> m_extension_list;
	VkCommandPool m_commandPool;

	void add_validation_layer(VkDeviceCreateInfo& createInfo);
	bool check_validation_layer_valid();
	bool check_device_extension_valid(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void create_command_pool();
	//--------------------
	// Physical device:
	//--------------------
	//The graphics card that we'll end up selecting will be stored 
	//in a VkPhysicalDevice handle that is added as a new class member.
	//This object will be implicitly destroyed when the VkInstance is destroyed, so we won't need to do anything new in the cleanup function.
	VkPhysicalDevice				m_physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		m_physical_device_properties{};
	VkPhysicalDeviceFeatures		m_physical_device_features{}; //init by vk_false
	std::vector <VkQueueFamilyProperties> m_physical_queuefamily_properties;
	void get_suitable_queuefamily(VkPhysicalDevice device, QueueFamilyIndex& queue_index);


	//--------------------
	// Other setting :
	//--------------------
	const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
	const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDebugUtilsMessengerEXT m_debugMessenger;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
};