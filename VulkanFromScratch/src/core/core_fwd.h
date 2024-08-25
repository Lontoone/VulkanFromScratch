#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
//#include "./core/sturcture_h.h"

#include "./core/component.hpp"
#include "./core/gameobject.hpp"
#include "./core/core_instance.hpp"
#include "./core/window.hpp"
#include "./core/swapchain.hpp"
#include "./core/pipeline.hpp"
#include "./core/renderer.hpp"
#include "./core/model.hpp"
#include "./core/image.hpp"
#include "./core/transformObject.hpp" 


//#include "core/transformObject.hpp"
//----------------------------
// Main class
//----------------------------
class CoreInstance;
class Component;
class Image;
class TransformObject;
class GameObject;

//----------------------------
// Helper
//----------------------------
uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmd_pool);
void endSingleTimeCommands(CoreInstance& core_instance, VkCommandBuffer commandBuffer);
void createBuffer(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory);
