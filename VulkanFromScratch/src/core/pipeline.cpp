#include "pipeline.hpp"
#include "model.hpp"
GraphicsPipeline::GraphicsPipeline(CoreInstance& _core, SwapChain& swapchain) : m_core_instance{ _core }, m_swapchain{swapchain}
{
	load_shaders();
}

GraphicsPipeline::~GraphicsPipeline()
{
	cleanup();
}

void GraphicsPipeline::load_shaders()
{
	auto vertShaderCode = readFile("./src/shaders/simple_shader.vert.spv");
	auto fragShaderCode = readFile("./src/shaders/simple_shader.frag.spv");
	

	m_vert_shader_module = createShaderModule(vertShaderCode);
	m_frag_shader_module = createShaderModule(fragShaderCode);


}

VkShaderModule GraphicsPipeline::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	// the bytecode pointer is a uint32_t pointer rather than a char pointer.
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_core_instance.get_device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void GraphicsPipeline::cleanup()
{
	vkDestroyShaderModule(m_core_instance.get_device(), m_vert_shader_module, nullptr);
	vkDestroyShaderModule(m_core_instance.get_device(), m_frag_shader_module, nullptr);
	vkDestroyPipelineLayout(m_core_instance.get_device() , m_pipeline_layout, nullptr);
	//vkDestroyRenderPass(m_core_instance.get_device(), m_renderPass, nullptr);
	vkDestroyPipeline(m_core_instance.get_device(), m_graphicsPipeline, nullptr);
	
}

/*
 A Vulkan pipeline cannot be created for multiple render passes. 
 Each graphics pipeline in Vulkan is created for a specific render 
 pass and subpass. This is because the pipeline state includes information 
 that is specific to the render pass, such as the layout of attachments and 
 the operations performed in the subpass.
*/
void GraphicsPipeline::create_pipleine( 
	VkRenderPass renderpass , 
	std::vector<VkDescriptorSetLayout>* descriptors )
{
	//-------------------
	// 	   Vert Stage
	//-------------------
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_vert_shader_module;
	vertShaderStageInfo.pName = "main";

	//-------------------
	// 	   Frag Stage
	//-------------------
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_frag_shader_module;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	auto vert_binding_dscp = Model::Vertex::get_binding_description();
	auto attr_binding_dscp = Model::Vertex::get_attribute_descriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vert_binding_dscp; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr_binding_dscp.size());
	vertexInputInfo.pVertexAttributeDescriptions = attr_binding_dscp.data(); // Optional

	//---------------------- fixed-function ------------------------



	//-------------------
	// 	   Pipeline assembly
	//-------------------
	// Describes two things: what kind of geometry will be drawn from the vertices and if 
	// primitive restart should be enabled.
	//   -> Draw point ? line? triangle?
	//   -> Strip mode enable?
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	

	//-------------------
	// 	   Viewport
	//-------------------
	// Viewport(s) and scissor rectangle(s) can either be specified as a static part of the 
	// pipeline or as a dynamic state set in the command buffer. 

	// Without dynamic state, the viewport and scissor rectangle need to be set in the pipeline
	// using the VkPipelineViewportStateCreateInfo struct. This makes the viewport and scissor 
	// rectangle for this pipeline immutable. 

	//--- Static:
	/*
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_width;
	viewport.height = (float)m_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VkExtent2D{m_width, m_height};

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	*/

	//-------------------
	// 	   Dynamic
	//-------------------
	// While most of the pipeline state needs to be baked into the pipeline state, a limited amount of
	// the state can actually be changed without recreating the pipeline at draw time. Examples are the 
	// size of the viewport, line width and blend constants.
	// This will cause the configuration of these values to be ignored and you will be able (and required) 
	// to specify the data at drawing time. 
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;


	//-------------------
	// 	   Rasterizer
	//-------------------
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	// If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the 
	// rasterizer stage. This basically disables any output to the framebuffer.
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;

	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	//-------------------
	// 	   Multisampling
	//-------------------
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	//-------------------
	// 	   Color blending
	//-------------------
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// If you want to use the second method of blending (bitwise combination), then you should 
	// set logicOpEnable to VK_TRUE. The bitwise operation can then be specified in the logicOp field.
	// Note that this will automatically disable the first method, as if you had set blendEnable to 
	// VK_FALSE for every attached framebuffer!
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	//-------------------
	// 	  Create Pipeline Layout
	//-------------------
	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	/*
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	*/
	//pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	//pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	//pipelineLayoutInfo.setLayoutCount = 1;//static_cast<uint32_t>( descriptors.size()); // Optional
	//pipelineLayoutInfo.pSetLayouts = &descriptors; // Optional
	
	pipelineLayoutInfo.setLayoutCount = descriptors->size();
	pipelineLayoutInfo.pSetLayouts = descriptors->data();

	if (vkCreatePipelineLayout(
		m_core_instance.get_device(), &pipelineLayoutInfo, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	//-------------------
	// 	  Create Pipeline 
	//-------------------
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages ;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipeline_layout;
	pipelineInfo.renderPass = renderpass;
	// It is also possible to use other render passes with this pipeline instead of
	// this specific instance, but they have to be compatible with renderPass.
	pipelineInfo.subpass = 0;

	// Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline.
	/*
	 You can either specify the handle of an existing pipeline with basePipelineHandle or reference 
	 another pipeline that is about to be created by index with basePipelineIndex. Right now there 
	 is only a single pipeline, so we'll simply specify a null handle and an invalid index. These 
	 values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags 
	 field of VkGraphicsPipelineCreateInfo.
	https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
	*/
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	//pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(
		m_core_instance.get_device(),
		VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}



