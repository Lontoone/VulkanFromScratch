#include "swapchain.hpp"
#include <stdexcept>
#include <iostream>
//#include "core_fwd.h"


SwapChain::SwapChain(CoreInstance& core_instance, const unsigned int w, const unsigned int h) : 
	m_core_instance{ core_instance },m_width{ w }, m_height{h}
{
	//---------------------------
	// 	   Get Support
	//---------------------------
	query_support(); // Query all the supported settings. and update to m_support_details
	VkSurfaceFormatKHR  surface_format = choose_swap_surface_format(m_support_details.formats);
	VkPresentModeKHR  present_mode = choose_swap_present_mode(m_support_details.presentModes);
	VkExtent2D  extent = choose_swap_extent(m_support_details.capabilities ,w,h);  // Todo... need width and height

	create_swap_chain(surface_format , present_mode, extent);
	create_images();
	create_image_view(); 
	create_syncobjects();
	create_depth_buffer();

}

SwapChain::~SwapChain()
{
	vkDestroySwapchainKHR(m_core_instance.get_device(), m_swapchain, nullptr);
}

void SwapChain::create_swap_chain(const VkSurfaceFormatKHR& surface_format, const VkPresentModeKHR& present_mode, const VkExtent2D& extent)
{
	//------------------------
	// 	   Set image count
	//------------------------
	m_image_count = m_support_details.capabilities.minImageCount + 1; // > 1 is better
	// (optinal ): set to max image count
	if (m_support_details.capabilities.maxImageCount > 0 && m_image_count > m_support_details.capabilities.maxImageCount) {
		// 0 means : no limit
		m_image_count = m_support_details.capabilities.maxImageCount;
	}
	m_swapChain_images.resize(m_image_count);
	m_swapChain_image_format = surface_format.format;
	m_swapChain_extent = extent;

	//---------------------------
	// 	   Create Info
	//---------------------------
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_core_instance.get_surface();
	createInfo.minImageCount = m_image_count;
	createInfo.imageFormat = surface_format.format;
	createInfo.imageColorSpace = surface_format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;   // how many images per-layer. => 2D only need 1
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //directly to render. or VK_IMAGE_USAGE_TRANSFER_DST_BIT  for post process first

	auto indices = m_core_instance.get_queuefailmy_indexs();
	uint32_t queueFamilyIndices[] = { indices->graphic_queuefamily_index.value(), indices->present_queuefamily_index.value() };


	if (indices->graphic_queuefamily_index != indices->present_queuefamily_index) {
		//VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.
		//Concurrent mode requires you to specify in advance between which queue families ownership will be shared using the
		//queueFamilyIndexCount and pQueueFamilyIndices parameters.

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		//VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership 
		//must be explicitly transferred before using it in another queue family. This option offers the best performance.
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	//(supportedTransforms in capabilities), like a 90 degree clockwise rotation or horizontal flip. 
	createInfo.preTransform = m_support_details.capabilities.currentTransform;
	//The compositeAlpha field specifies if the alpha channel should be used for blending with other
	//windows in the window system. 
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // simply ignore the alpha channel
	createInfo.presentMode = present_mode;
	//we don't care about the color of pixels that are obscured, for example because another window is in front of them.
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	try {
		if (vkCreateSwapchainKHR(m_core_instance.get_device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}
	}
	catch (std::exception& e) {
		std::cout << "exception: " << e.what() << "\n";
	}

}


void SwapChain::query_support()
{
	// Get physical support details
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		m_core_instance.get_physical_device(),
		m_core_instance.get_surface(),
		&m_support_details.capabilities);

	// Get the supported surface formats.
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		m_core_instance.get_physical_device(), m_core_instance.get_surface(), &formatCount, nullptr);

	if (formatCount != 0) {
		m_support_details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			m_core_instance.get_physical_device(), m_core_instance.get_surface(), &formatCount,
			m_support_details.formats.data());
	}

	// Get the supported presentation modes.
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_core_instance.get_physical_device(),
		m_core_instance.get_surface(),
		&presentModeCount, nullptr);

	if (presentModeCount != 0) {
		m_support_details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			m_core_instance.get_physical_device(), m_core_instance.get_surface(),
			&presentModeCount, m_support_details.presentModes.data());
	}

	// Simple check:
	if (m_support_details.presentModes.empty()) {
		throw std::runtime_error("Faild to create swapchain");
	}

	
}

VkSurfaceFormatKHR SwapChain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Perfer SRGB for better precision
	// https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities , const unsigned int w , const unsigned int h)
{
	
		VkExtent2D actualExtent = {
			w,
			h
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
}

// ToDo: Use helper functions
void SwapChain::create_image_view()
{
	// An image view is quite literally a view into an image.
	// It describes how to access the image and which part of 
	// the image to access, for example if it should be treated 
	// as a 2D texture depth texture without any mipmapping levels.
	m_swapChain_image_views.resize(m_swapChain_images.size());

	VkImageViewCreateInfo createInfo{};
	for (size_t i = 0; i < m_swapChain_images.size(); i++) {
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChain_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChain_image_format;
		
		// The components field allows you to swizzle the color channels around. 
		//https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkComponentSwizzle.html
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// The subresourceRange field describes what the image's purpose is and which part of
		// the image should be accessed. Our images will be used as color targets without any
		// mipmapping levels or multiple layers.
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_core_instance.get_device(), &createInfo, nullptr, &m_swapChain_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void SwapChain::create_images()
{

	//-----------------
	// 	   Create Images
	//-----------------	
	//vkGetSwapchainImagesKHR(m_core_instance.get_device(), m_swapchain, &m_image_count, nullptr);
	//swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_core_instance.get_device(), m_swapchain, &m_image_count, m_swapChain_images.data());

}

void SwapChain::clean()
{
	for (auto imageView : m_swapChain_image_views) {
		vkDestroyImageView(m_core_instance.get_device(), imageView, nullptr);
	}
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(	m_core_instance.get_device(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(	m_core_instance.get_device(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(		m_core_instance.get_device(), m_inFlightFences[i], nullptr);	
	}

	// command buffers are freed for us when we free the command pool.
}

void SwapChain::create_syncobjects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// Create semaphore
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create fence
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Default is signaled. The first vkWaitforfence will not wait.
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(m_core_instance.get_device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_core_instance.get_device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_core_instance.get_device(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void SwapChain::create_depth_buffer()
{
	auto depth_format = findDepthFormat(m_core_instance.get_physical_device());
	
	bool has_stencil = hasStencilComponent(depth_format);

	createImage(m_core_instance.get_device() , m_core_instance.get_physical_device(),
		m_width, m_height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(m_core_instance.get_device(), depthImage, depth_format , VK_IMAGE_ASPECT_DEPTH_BIT);
	
	//transitionImageLayout(m_core_instance, depthImage, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}
