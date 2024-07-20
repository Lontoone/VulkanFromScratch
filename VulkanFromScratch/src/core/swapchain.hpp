#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include "./core/core_instance.hpp"
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
// Vulkan does not have the concept of a "default framebuffer".
class SwapChain{
public :
    SwapChain(CoreInstance& core_instance, const unsigned int w, const unsigned int h);
    ~SwapChain(); //todo....

    // With 3 or more frames in flight, the CPU could get ahead of the GPU, adding frames of latency. 
    static const int MAX_FRAMES_IN_FLIGHT = 2;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    void create_swap_chain(const VkSurfaceFormatKHR &format, const VkPresentModeKHR & present_mode , const VkExtent2D& extent);
    VkFormat get_format() const { return m_swapChain_image_format; }
    const std::vector<VkImageView>& get_image_views() const {return m_swapChain_image_views; }
    inline const unsigned int _width() const{ return m_width; }
    inline const unsigned int _height() const { return m_height; }
    inline const unsigned int current_frame() const { return m_currentFrame; }
    inline const VkSwapchainKHR swap_chain() const { return m_swapchain; }
    inline VkFence& get_fence() { return m_inFlightFences[m_currentFrame]; }  // todo: auto fence pool
    inline const VkSemaphore get_avaliable_semaphore(){ return m_imageAvailableSemaphores[m_currentFrame]; }  // todo: auto fence pool
    inline const VkSemaphore get_finish_semaphore() { return m_renderFinishedSemaphores[m_currentFrame]; }  // todo: auto fence pool

    //VkSemaphore m_imageAvailableSemaphore;
    //VkSemaphore m_renderFinishedSemaphore;
    //VkFence m_inFlightFence;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    void update_frame_count() { (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; };
private:
    const unsigned int m_width;
    const unsigned int m_height;
    CoreInstance&               m_core_instance;
    SwapChainSupportDetails     m_support_details{};
    uint32_t                    m_image_count;
    uint32_t                    m_currentFrame = 0;
    VkSwapchainKHR              m_swapchain;
    //-----------------
    //   Images
    //-----------------
    std::vector<VkImage>        m_swapChain_images;
    VkFormat                    m_swapChain_image_format;
    VkExtent2D                  m_swapChain_extent;
    std::vector<VkImageView>    m_swapChain_image_views;

    

    void query_support();
    void create_images();
    void create_image_view();
    void clean();
    void create_syncobjects();
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const unsigned int w, const unsigned int h);
};

// ToDo:
//  * Swap chain recreation
//      https://vulkan-tutorial.com/en/Drawing_a_triangle/Swap_chain_recreation