#pragma once
#include <cstddef>
#include <vulkan/vulkan.h>
//#include "./core/core_fwd.h"
class Component
{
public:
	virtual VkDescriptorSetLayout get_descriptorset_layout() { return NULL; };
	virtual void update(FrameUpdateData& updateData ) {  };
	//virtual void bind(const VkCommandBuffer& cmdBuf) {};
	
private:
	
};