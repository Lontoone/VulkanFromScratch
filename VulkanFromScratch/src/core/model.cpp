#include "model.hpp"
#include <stdexcept>

Model::Model(CoreInstance& _core, VkPipeline pipeline) : m_core_instance{ _core } , m_pipeline{pipeline}
{	
	create_vertexBuffer();
	create_indexBuffer();
}

Model::~Model()
{
	vkDestroyBuffer(m_core_instance.get_device(), m_vertexBuffer, nullptr);
	vkDestroyBuffer(m_core_instance.get_device(), m_vertexBuffer, nullptr);
	vkDestroyBuffer(m_core_instance.get_device(), m_indexBuffer, nullptr);
	vkFreeMemory(m_core_instance.get_device(), m_indexBufferMemory, nullptr);
	vkFreeMemory(m_core_instance.get_device(), m_vertexBufferMemory, nullptr);

}

void Model::update(FrameUpdateData& update_data)
{
	this->bind(update_data.m_cmdbuffer);
	this->draw(update_data.m_cmdbuffer);
}


void Model::bind(const VkCommandBuffer& cmdBuf )
{
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
	// the possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
	vkCmdBindIndexBuffer(cmdBuf , m_indexBuffer , 0 , VK_INDEX_TYPE_UINT16); //you can only have a single index buffer
}

void Model::draw(const VkCommandBuffer& cmdBuf)
{
	//vkCmdDraw(cmdBuf, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	vkCmdDrawIndexed(cmdBuf, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Model::create_vertexBuffer()
{
	uint32_t buffer_size = sizeof(vertices[0]) * vertices.size();
	createBuffer(
		m_core_instance.get_device(),
		m_core_instance.get_physical_device(),
		buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_vertexBuffer,
		m_vertexBufferMemory);
	/*
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(
		m_core_instance.get_device(),
		&bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	//----------------
	//		Memory
	//----------------
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_core_instance.get_device(), m_vertexBuffer, &memRequirements);

	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(m_core_instance.get_device(), &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(m_core_instance.get_device(), m_vertexBuffer, m_vertexBufferMemory, 0);
	}
	*/
	//----------------
	//		Dtat mapping
	//----------------
	{
		void* data; // pointer to the mapped memory
		vkMapMemory(m_core_instance.get_device(), m_vertexBufferMemory, 0, buffer_size, 0, &data);
		// The driver may not immediately copy the data into the buffer memory, for example because of caching. 
		// It is also possible that writes to the buffer are not visible in the mapped memory yet. There are two
		// ways to deal with that problem:
		//		* Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		//		* Call vkFlushMappedMemoryRanges after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges 
		//		  before reading from the mapped memory
		//memcpy(data, vertices.data(), (size_t)bufferInfo.size);
		memcpy(data, vertices.data(), buffer_size);
		vkUnmapMemory(m_core_instance.get_device(), m_vertexBufferMemory);

		// Flushing memory ranges or using a coherent memory heap means that the driver will be aware of our writes to the buffer,
		// but it doesn't mean that they are actually visible on the GPU yet. The transfer of data to the GPU is an operation that
		// happens in the background and the specification simply tells us that it is guaranteed to be complete as of the next call
		// to vkQueueSubmit.
	}

}

void Model::create_indexBuffer()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(indices[0]) * indices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(
		m_core_instance.get_device(),
		&bufferInfo, nullptr, &m_indexBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	//----------------
	//		Memory
	//----------------
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_core_instance.get_device(), m_indexBuffer, &memRequirements);
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(m_core_instance.get_device(), &allocInfo, nullptr, &m_indexBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(m_core_instance.get_device(), m_indexBuffer, m_indexBufferMemory, 0);
	}

	//----------------
	//		Dtat mapping
	//----------------
	{
		void* data; // pointer to the mapped memory
		vkMapMemory(m_core_instance.get_device(), m_indexBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(m_core_instance.get_device(), m_indexBufferMemory);
	}
}

uint32_t Model::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	/*
	The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes and memoryHeaps. 
	Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for when
	VRAM runs out. The different types of memory exist within these heaps. Right now we'll only 
	concern ourselves with the type of memory and not the heap it comes from, but you can imagine 
	that this can affect performance.
	*/
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_core_instance.get_physical_device(), &memProperties);
	// memoryTypeCount : The number of memory types available.
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}


// ToDo:
//	Staging memory
//		https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer