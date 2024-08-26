#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include "./core/sturcture_h.h"
#include "./core/core_instance.hpp"

//#include "core/transformObject.hpp"
//----------------------------
// Main class
//----------------------------
class CoreInstance;
class Component;
class Image;
class TransformObject;
class GameObject;

/*
//----------------------------
// Helper
//----------------------------
static uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmd_pool);
static void endSingleTimeCommands(CoreInstance& core_instance, VkCommandBuffer commandBuffer);
static void createBuffer(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory);


static VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


#include "./core/component.hpp"
#include "./core/gameobject.hpp"
#include "./core/window.hpp"
#include "./core/swapchain.hpp"
#include "./core/pipeline.hpp"
#include "./core/renderer.hpp"
#include "./core/model.hpp"
#include "./core/image.hpp"
#include "./core/transformObject.hpp" 

*/