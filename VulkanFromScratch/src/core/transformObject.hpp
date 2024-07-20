#pragma once
#include "core/core_fwd.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
//#include "helper/storage.hpp"
//#include "core/core_instance.hpp"
//#include "vulkan/vulkan.h"
#include <chrono>
#include <vector>

#include <stdexcept>

class TransformObject: public Component {
public:
    TransformObject(CoreInstance& core);
    ~TransformObject();
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    void updateUniformBuffer(uint32_t currentImage);
    void bind(VkCommandBuffer& cmdbuffer, unsigned int currentFrame , VkPipelineLayout& pipeline_layout);

    VkDescriptorSetLayout      m_descriptorSetLayout;
    VkDescriptorSetLayout      get_descriptorset_layout() override;
private:
    void cleanup();

    CoreInstance& m_core_instance;
    void createDescriptorSetLayout();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    //VkPipelineLayout            m_pipelineLayout;
    VkDescriptorPool            m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    
    std::vector<VkBuffer>       m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*>          m_uniformBuffersMapped;

    int MAX_FRAMES_IN_FLIGHT = 2; //temp
};