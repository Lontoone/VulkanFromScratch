#pragma once

#include <vulkan/vulkan.h>
#include "./helper/file_loader.hpp"
#include <core/core_instance.hpp>
#include "swapchain.hpp"

class GraphicsPipeline {
public:
	GraphicsPipeline(CoreInstance& _core , SwapChain& swapchain );
	~GraphicsPipeline();

	void load_shaders();
	//inline const auto get_renderPass()const { return m_renderPass; }
	inline const VkPipeline& get_pipeline() const { return m_graphicsPipeline;}
	inline VkPipelineLayout& get_layout() { return m_pipeline_layout; }
	
	void create_pipleine( VkRenderPass renderpass ,
		std::vector<VkDescriptorSetLayout>* descriptors);
private:
	VkShaderModule createShaderModule(const std::vector<char>& code);
	CoreInstance& m_core_instance;
	SwapChain& m_swapchain;
	VkShaderModule m_vert_shader_module;
	VkShaderModule m_frag_shader_module;
	VkPipelineLayout m_pipeline_layout;
	//VkRenderPass m_renderPass;
	VkPipeline m_graphicsPipeline;
	//unsigned int m_width;
	//unsigned int m_height;

	void cleanup();

	//void create_renderPass();
};