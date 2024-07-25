#pragma once
#include "core/core_fwd.h"
//#include "core/component.hpp"
#include <vector>
#include <vulkan/vulkan.h>
class GameObject
{
public:
	std::vector<Component*> m_components;
	void add_component(Component* comp);
	std::vector<VkDescriptorSetLayout>* get_all_descriptorLayouts();
	std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

	//void update_descriptor();
	//void bind(const VkCommandBuffer& cmdBuf);
	void execute(FrameUpdateData& frame_data);
	void execute_before_frame(FrameUpdateData& frame_data);
	void execute_after_frame(FrameUpdateData& frame_data);
private:

};
