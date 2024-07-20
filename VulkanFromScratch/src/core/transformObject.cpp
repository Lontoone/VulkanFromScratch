#include "transformObject.hpp"


TransformObject::TransformObject(CoreInstance& core) : m_core_instance{core}
{

    createDescriptorSetLayout();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    
}

TransformObject::~TransformObject()
{
    cleanup();
}

VkDescriptorSetLayout TransformObject::get_descriptorset_layout()
{
    return m_descriptorSetLayout;
}

void TransformObject::cleanup()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(
            m_core_instance.get_device(),
            m_uniformBuffers[i], nullptr);
        vkFreeMemory(
            m_core_instance.get_device(),
            m_uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorSetLayout(
        m_core_instance.get_device(),
        m_descriptorSetLayout,
        nullptr);

    vkDestroyDescriptorPool(m_core_instance.get_device(),  m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_core_instance.get_device(), m_descriptorSetLayout, nullptr);

}

void TransformObject::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // only relevant for image sampling 
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(
        m_core_instance.get_device(),
        &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }



    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
}

void TransformObject::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(
            m_core_instance.get_device(),
            m_core_instance.get_physical_device(),
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBuffers[i],
            m_uniformBuffersMemory[i]);

        // to get a pointer to which we can write the data later on.
        // persistent mapping
        vkMapMemory(
            m_core_instance.get_device(),
            m_uniformBuffersMemory[i],
            0,
            bufferSize,
            0,
            &m_uniformBuffersMapped[i]);
    }

}

void TransformObject::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    ubo.proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
    // GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
    ubo.proj[1][1] *= -1;  // drawn in counter-clockwise order instead of clockwise order!!

    // directly update to buffer memory
    memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

}

void TransformObject::createDescriptorPool()
{
    // Descriptor sets can't be created directly, they must be allocated from a
    // pool like command buffers. 
    // You need to specify the descriptor pool to allocate descriptor sets!

    // We will allocate one of these descriptors for every frame. 
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);


    if (vkCreateDescriptorPool(
        m_core_instance.get_device(),
        &poolInfo,
        nullptr,
        &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void TransformObject::createDescriptorSets()
{
    // create one descriptor set for each frame in flight , all with the same layout. 
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    // You don't need to explicitly clean up descriptor sets, because 
    // they will be automatically freed when the descriptor pool is destroyed.
    // 
    // Unfortunately we do need all the copies of the layout because the next 
    // function expects an array matching the number of sets.
    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_core_instance.get_device(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // copy n times
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // descriptors can be arrays
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        //  It's possible to update multiple descriptors at once in an array,
        //  starting at index dstArrayElement. The descriptorCount field specifies
        //  how many array elements you want to update.
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.pBufferInfo = &bufferInfo;  //for descriptors that refer to buffer data
        descriptorWrite.pImageInfo = nullptr; //  for descriptors that refer to image data,
        descriptorWrite.pTexelBufferView = nullptr; // for descriptors that refer to buffer views
        
        //  It accepts two kinds of arrays as parameters: 
        //          an array of VkWriteDescriptorSet and 
        //          an array of VkCopyDescriptorSet. 
        vkUpdateDescriptorSets(m_core_instance.get_device(), 1, &descriptorWrite, 0, nullptr);
        
    }
   
}

void TransformObject::bind(VkCommandBuffer& cmdbuffer , unsigned int currentFrame , VkPipelineLayout& pipeline_layout)
{
    vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,   //Where to bind
        0,  // index of the first descriptor set,
        1,  // the number of sets to bind
        &m_descriptorSets[currentFrame], //array of set to bind
        0, nullptr);
}
