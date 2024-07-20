#pragma once
#include "core/core_fwd.h"
#include "helper/storage.hpp"
int main() {

	DisplayWindow main_window {};
	CoreInstance coreInstance{ *main_window.get_window() };	
	SwapChain swapchain{coreInstance, main_window.SCR_WIDTH , main_window.SCR_HEIGHT};
	GraphicsPipeline pipeline{coreInstance , swapchain };
	Renderer renderer{coreInstance , swapchain};
	TransformObject transform_obj{ coreInstance };
	GameObject gameobject{};
	
	gameobject.add_component(&transform_obj);


	Image img{ coreInstance };
	img.load_texture("./assets/texture.jpg");
	/*
	*/

	//pipeline.create_pipleine(renderer.get_renderPass() , transform_obj.m_descriptorSetLayout);
	pipeline.create_pipleine(
		renderer.get_renderPass(),
		gameobject.get_all_descriptorLayouts()
	);
	Model model{coreInstance , pipeline.get_pipeline()};
	renderer.create_frameBuffer(swapchain, renderer.get_renderPass());
	//renderer.create_commnadPool();	
	renderer.create_commandBuffer();
	//renderer.record_commandBuffer(0 , pipeline.get_pipeline());


	while (main_window.is_window_alive())
	{
		glfwPollEvents();
		renderer.reset_renderpass();
		renderer.begin_commandBuffer();		
		transform_obj.updateUniformBuffer(swapchain.current_frame());
		transform_obj.bind(
			renderer.get_current_cmdbuffer(),
			swapchain.current_frame(),
			pipeline.get_layout()) ;

		model.bind(renderer.get_current_cmdbuffer());
		model.draw(renderer.get_current_cmdbuffer());

		renderer.end_render();
		renderer.draw_frame();
	}

}


//-----------------------------------------
// Others:
// Learn makefile : https://vulkan-tutorial.com/en/Development_environment


//https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_pool_and_sets