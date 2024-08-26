#include "renderer.hpp"
#include <stdexcept>

Renderer::Renderer(CoreInstance& core_instance, SwapChain& swapchain) :m_core_instance{ core_instance }, m_swapchain{swapchain}
{
    create_renderPass();
    create_frameBuffer(m_swapchain , m_renderpass);
    create_commandBuffer();
}

Renderer::~Renderer()
{
    cleanup();
}

void Renderer::cleanup()
{
    for (auto framebuffer : m_swapChain_framebuffers) {
        vkDestroyFramebuffer(m_core_instance.get_device(), framebuffer, nullptr);
    }
    vkDestroyCommandPool(m_core_instance.get_device(), m_core_instance.cmd_pool(), nullptr);
    vkDestroyRenderPass(m_core_instance.get_device(), m_renderpass, nullptr);
}

void Renderer::create_frameBuffer(SwapChain& swapchain, VkRenderPass renderPass )
{
    m_renderpass = renderPass;
    // Specify with which renderPass the framebuffer needs to be compatible, which 
    // roughly means that they use the same number and type of attachments.

	m_swapChain_framebuffers.resize(swapchain.get_image_views().size());

    for (size_t i = 0; i < swapchain.get_image_views().size(); i++) {
        VkImageView attachments[] = {
            swapchain.get_image_views()[i] , 
            swapchain.get_depth_img_view()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;  //attachments.size
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain._width();
        framebufferInfo.height = swapchain._height();
        framebufferInfo.layers = 1; //Our swap chain images are single images, so the number of layers is 1.

        if (vkCreateFramebuffer(m_core_instance.get_device(), &framebufferInfo, nullptr, &m_swapChain_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
/*
void Renderer::create_commnadPool()
{
    // Command buffers are executed by submitting them on one of the device queues.
    // Each command pool can only allocate command buffers that are submitted on a single type of queue. 

    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;   
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_core_instance.get_queuefailmy_indexs()->graphic_queuefamily_index.value();

    if (vkCreateCommandPool(m_core_instance.get_device(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}
*/

void Renderer::create_commandBuffer()
{
    m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_core_instance.cmd_pool();
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
    /*
    The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
        *VK_COMMAND_BUFFER_LEVEL_PRIMARY: 
                Can be submitted to a queue for execution, but cannot be called from other command buffers.
        *VK_COMMAND_BUFFER_LEVEL_SECONDARY: 
                Cannot be submitted directly, but can be called from primary command buffers.
    We won't make use of the secondary command buffer functionality here, but you can imagine that it's 
    helpful to reuse common operations from primary command buffers.
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;    

    if (vkAllocateCommandBuffers(m_core_instance.get_device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

//void Renderer::begin_commandBuffer(uint32_t imageIndex , VkPipeline graphicsPipeline)
void Renderer::begin_commandBuffer()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    /*
    The flags parameter specifies how we're going to use the command buffer. The following values are available:

        * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
        * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
        * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    None of these flags are applicable for us right now.
    */
    beginInfo.flags = 0; // Optional
    // pInheritanceInfo is only relevant for secondary command buffers. 
    // It specifies which state to inherit from the calling primary command buffers.
    beginInfo.pInheritanceInfo = nullptr; // Optional 

    // If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will 
    // implicitly reset it. It's not possible to append commands to a buffer at a later time.
    auto current_frame = m_swapchain.current_frame();
    if (vkBeginCommandBuffer(m_commandBuffers[current_frame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }


    // We created a framebuffer for each swap chain image where it is specified as a color attachment.
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderpass;
    renderPassInfo.framebuffer = m_swapChain_framebuffers[m_imageIndex];

    // The render area defines where shader loads and stores will take place. 
    // The pixels outside this region will have undefined values. It should match 
    // the size of the attachments for best performance.
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = VkExtent2D{m_swapchain._width() , m_swapchain._height() };
    
    // Define clear color
    std::vector<VkClearValue>clearColor{ VkClearValue(), VkClearValue ()};
    clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearColor[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());;
    renderPassInfo.pClearValues = clearColor.data();
    


    /*
        * VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
        * VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
    */
    vkCmdBeginRenderPass(m_commandBuffers[current_frame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //---------------------------    
    //      Draw Commnad begin here....
    // All of the functions that record commands can be recognized by their vkCmd prefix. 
    // They all return void, so there will be no error handling until we've finished recording.
    //---------------------------
    //vkCmdBindPipeline(m_commandBuffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    /*    
        we did specify viewport and scissor state for this pipeline to be dynamic. So we need to set 
        them in the command buffer before issuing our draw command:    
    */
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain._width());
    viewport.height = static_cast<float>(m_swapchain._height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffers[current_frame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = VkExtent2D{ m_swapchain._width() ,m_swapchain._height() };
    vkCmdSetScissor(m_commandBuffers[current_frame], 0, 1, &scissor);


    // You can draw now....
}

// If your fragment shader have multiple output target, modify this!!!
// https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes
void Renderer::create_renderPass()
{   
    //-------------------
    // 	   Attachment Description
    //-------------------
    VkAttachmentDescription colorAttachment{};
    //should match the format of the swap chain images
    colorAttachment.format = m_swapchain.get_format();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    /*
    The loadOp and storeOp determine what to do with the data in the attachment before rendering and
    after rendering. We have the following choices for loadOp:

        VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
        VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
        VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them

    In our case we're going to use the clear operation to clear the framebuffer to black before drawing
    a new frame.

    There are only two possibilities for the storeOp:
        VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
        VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
    We're interested in seeing the rendered triangle on the screen, so we're going with the store operation here.
    */

    // Our application won't do anything with the stencil buffer
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    /*
    Some of the most common layouts are:
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
    */
    // Images need to be transitioned to specific layouts that are suitable for the operation that they're
    // going to be involved in next.

    /*
    Using VK_IMAGE_LAYOUT_UNDEFINED for initialLayout means that we don't care what previous layout the image was in.
    The caveat of this special value is that the contents of the image are not guaranteed to be preserved, but that
    doesn't matter since we're going to clear it anyway. We want the image to be ready for presentation using the swap
    chain after rendering, which is why we use VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayout.
    */
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //which layout the image will have before the render pass begins. 
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //-------------------
    // 	   Pipeline Reference
    //-------------------
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Our array consists of a single VkAttachmentDescription, so its index is 0.
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //--------------------------
   //      Depth
   //--------------------------
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(m_core_instance.get_physical_device()); // The format should be the same as the depth image itself.
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // it will not be used after drawing has finished.
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //we don't care about the previous depth contents
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



    VkSubpassDescription subpass{};
    // The index of the attachment in this array is directly referenced from the fragment shader with the 
    // layout(location = 0) out vec4 outColor directive!
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
   
    // Subpasses in a render pass automatically take care of image layout transitions. 
    // These transitions are controlled by subpass dependencies, which specify memory 
    // and execution dependencies between subpasses.

    VkSubpassDependency dependency{};
    // The dstSubpass must always be higher than srcSubpass to prevent cycles in the dependency 
    // graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass depending
    dependency.dstSubpass = 0;  // first pass
    /*
        It can be used to ensure that the rendering operations in one subpass are completed
        before starting the rendering operations in the next subpass that also writes to the
        same color attachment.
    */
    // specify the operations to wait on and the stages in which these operations occur. 
    // We need to wait for the swap chain to finish reading from the image before we can access it.
    // This can be accomplished by waiting on the color attachment output stage itself.
    /*
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //specifies the pipeline stage that produces the data.

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// indicating that the subsequent operations will also involve writing to color attachments.
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // indicates that the destination stage will involve writing to color attachments.
    */
    dependency.srcAccessMask = 0; // 0 means the dependency is not waiting for any specific memory access to complete.
    // Add VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT to perform depth test
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

   
    //--------------------------
    //      Render Pass
    //--------------------------    
    std::vector<VkAttachmentDescription> attachments{ colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderpassinfo{};
    renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassinfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
    renderpassinfo.pAttachments = attachments.data();
    renderpassinfo.subpassCount = 1;
    renderpassinfo.pSubpasses = &subpass;

    renderpassinfo.dependencyCount = 1;
    renderpassinfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_core_instance.get_device(), &renderpassinfo, nullptr, &m_renderpass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Renderer::draw_frame()
{
    unsigned int current_frame = m_swapchain.current_frame();
    /*
    At a high level, rendering a frame in Vulkan consists of a common set of steps:
        * Wait for the previous frame to finish
        * Acquire an image from the swap chain
        * Record a command buffer which draws the scene onto that image
        * Submit the recorded command buffer
        * Present the swap chain image
    */
        
    //record_commandBuffer(m_imageIndex, pipeline);

    //----------------
    //     Submit
    //----------------
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //----------------
    //     Wait before... semaphore
    //----------------
    // Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
    VkSemaphore waitSemaphores[] = { m_swapchain.get_avaliable_semaphore() };
    // Wait with writing colors to the image until it's available,
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores; //which semaphores to wait on before execution begins
    submitInfo.pWaitDstStageMask = waitStages; // in which stage(s) of the pipeline to wait.

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[current_frame];

    //----------------
    //     signal after... semaphore
    //----------------
    // Specify which semaphores to signal once the command buffer(s) have finished execution.
    VkSemaphore signalSemaphores[] = { m_swapchain.get_finish_semaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit the command buffer to the graphics queue
    //  The last parameter references an optional fence that will be signaled when the command buffers finish execution.
    //  This allows us to know when it is safe for the command buffer to be reused
    VkFence& fence = m_swapchain.get_fence();
    if (vkQueueSubmit(m_core_instance.graphic_queue(), 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }



    //-----------------------
    //      Submit result back to swapchain
    //-----------------------
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // specify which semaphores to wait on before presentation can happen
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapchain.swap_chain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;   // The swap chains to present images 
    presentInfo.pImageIndices = &m_imageIndex;

    // It allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful.
    presentInfo.pResults = nullptr; // Optional 
    // Submits the request to present an image to the swap chain
    vkQueuePresentKHR(m_core_instance.present_queue(), &presentInfo); 

    m_swapchain.update_frame_count();
}

void Renderer::reset_renderpass()
{
    // [1] Wait for previous fence
    //      The vkWaitForFences function takes an array of fences and waits on the 
    //      host for either any or all of the fences to be signaled before returning. 
    //          *VK_TRUE : wait for all fences,
    unsigned int current_frame = m_swapchain.current_frame();
    VkFence& fence = m_swapchain.get_fence();
    vkWaitForFences(m_core_instance.get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_core_instance.get_device(), 1, &fence);  // Rest


    
    // Signaled when the presentation engine is finished using the image. 
    vkAcquireNextImageKHR(m_core_instance.get_device(), m_swapchain.swap_chain(), UINT64_MAX,
        m_swapchain.get_avaliable_semaphore(), VK_NULL_HANDLE, &m_imageIndex);

    vkResetCommandBuffer(m_commandBuffers[current_frame], 0);
}

void Renderer::end_render()
{
    unsigned int current_frame = m_swapchain.current_frame();
    vkCmdEndRenderPass(m_commandBuffers[current_frame]);
    if (vkEndCommandBuffer(m_commandBuffers[current_frame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Renderer::bind(VkCommandBuffer& cmdBuf, VkPipelineLayout& pipeline_layout)
{
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,   //Where to bind
        1,  // index of the first descriptor set,
        1,  // the number of sets to bind
        &m_texture_image->m_descriptorSets, //array of set to bind
        0, nullptr);
}

VkDescriptorSetLayout Renderer::get_descriptorset_layout()
{
    return m_texture_image->get_descriptorsetLayout();
}

void Renderer::update(FrameUpdateData& updateData)
{
    this->bind(updateData.m_cmdbuffer , updateData.m_pipeline_layout);
}


