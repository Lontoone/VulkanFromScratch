#pragma once
#include "core/core_fwd.h"
#include "helper/storage.hpp"
int main() {

	DisplayWindow main_window {};
	CoreInstance coreInstance{ *main_window.get_window() };	
	SwapChain swapchain{coreInstance, main_window.SCR_WIDTH , main_window.SCR_HEIGHT};
	GraphicsPipeline pipeline{coreInstance , swapchain };
	Renderer forward_renderer_pass{coreInstance , swapchain};
	TransformObject transform_obj{ coreInstance };
	GameObject gameobject{};

	gameobject.add_component(&transform_obj);
	gameobject.add_component(&forward_renderer_pass);
	


	Image img{ coreInstance };
	img.load_texture("./assets/texture.jpg");
	forward_renderer_pass.m_texture_image = &img;

	pipeline.create_pipleine(
		forward_renderer_pass.get_renderPass(),
		gameobject.get_all_descriptorLayouts()
	);
	Model model{coreInstance , pipeline.get_pipeline()};
	gameobject.add_component(&model);
	
	while (main_window.is_window_alive())
	{
		glfwPollEvents();
		forward_renderer_pass.reset_renderpass();
		forward_renderer_pass.begin_commandBuffer();	

		FrameUpdateData update_data{ 
			swapchain.current_frame(),
			forward_renderer_pass.get_current_cmdbuffer(),
			pipeline.get_layout()
		};
		gameobject.execute(update_data);

		//transform_obj.updateUniformBuffer(swapchain.current_frame());
		/*
		transform_obj.bind(
			renderer.get_current_cmdbuffer(),
			swapchain.current_frame(),
			pipeline.get_layout()) ;
		renderer.bind(
			renderer.get_current_cmdbuffer(),			
			pipeline.get_layout());
		model.bind(renderer.get_current_cmdbuffer());
		model.draw(renderer.get_current_cmdbuffer());
		*/

		forward_renderer_pass.end_render();
		forward_renderer_pass.draw_frame();
	}

}


//-----------------------------------------
// Others:
// Learn makefile : https://vulkan-tutorial.com/en/Development_environment


//https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_pool_and_sets