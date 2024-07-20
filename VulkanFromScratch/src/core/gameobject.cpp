#include "gameobject.hpp"

void GameObject::add_component(Component* comp)
{
	this->m_components.emplace_back(comp);
}

std::vector<VkDescriptorSetLayout>* GameObject::get_all_descriptorLayouts()
{	
	for (int i = 0; i < m_components.size();++i) {
		m_descriptorSetLayouts.push_back( m_components[i]->get_descriptorset_layout());
	}

	return &m_descriptorSetLayouts;
}
