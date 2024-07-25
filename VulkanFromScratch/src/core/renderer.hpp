#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "core/core_fwd.h"
class Renderer : public Component{
	friend class Image;
public :
	Renderer(CoreInstance& core_instance , SwapChain& swapchain);
	~Renderer();

	//------------------
	//	Component class
	//------------------
	VkDescriptorSetLayout		get_descriptorset_layout() override;
	void						update(FrameUpdateData& updateData) override;
	
	//------------------
	//	Additional Property
	//------------------
	Image* m_texture_image;


	void cleanup();

	void create_frameBuffer(SwapChain& swapchain , VkRenderPass renderPass);	
	void create_commandBuffer();
	void create_renderPass();
	
	void draw_frame();
	void begin_commandBuffer();
	void reset_renderpass();
	void bind(VkCommandBuffer& cmdBuf , VkPipelineLayout& pipeline_layout);
	void end_render();
	
	inline const auto get_renderPass()const { return m_renderpass; } // Todo : split to small class
	inline VkCommandBuffer& get_current_cmdbuffer() { return m_commandBuffers[m_swapchain.current_frame()]; }
	

private:
	CoreInstance& m_core_instance;
	std::vector<VkFramebuffer> m_swapChain_framebuffers;
	SwapChain& m_swapchain;
	VkRenderPass m_renderpass;


	//VkCommandBuffer m_commandBuffer;
	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32_t m_imageIndex;

};