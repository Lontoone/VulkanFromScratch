#pragma once

//#include "core/core_fwd.h"
#include "helper/helper_functions.hpp"
#include "core/core_instance.hpp"


class  Image
{
	friend class Renderer;
public:
	Image(CoreInstance& core_instance);
	~Image() ;

	void load_texture(const char* path);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	// Temp :
	inline VkDescriptorSetLayout& get_descriptorsetLayout() 
		{return m_descriptorSetLayout;}

private:
	int m_width, m_height , m_channel;
	VkBuffer m_staging_buffer;
	VkDeviceMemory m_staging_buffer_memory;

	VkImage m_textureImage;
	VkDeviceMemory m_textureImage_memory;

	VkImageView m_texture_imageView;
	VkSampler m_texture_sampler;

	CoreInstance& m_core_instance;
	VkDescriptorSet m_descriptorSets;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;

	void create_texture();
	void createTextureImageView();
	void createTextureSampler();

	void createDescriptorSetLayout();
	void create_descriptor_pool();
	void create_descriptor();

	void cleanup();
	
};
