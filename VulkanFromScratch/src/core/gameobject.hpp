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
private:

};
