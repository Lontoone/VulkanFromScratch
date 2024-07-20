#include "core_instance.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <set>
#include <iostream>
#include <unordered_set>


//----------------------
//  Other function
//----------------------
bool isDeviceSuitable(VkPhysicalDevice device) {
    //Todo: check...
    
    // example:
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        //std::cout << " You are not using dedicated GPU!" << std::endl;
        printf(" You are not using dedicated GPU! \n");
    }


    // default
    return true;
}
// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

CoreInstance::CoreInstance(GLFWwindow& window)
{
    create_instance();
    setupDebugMessenger();
    create_surface(window);
    setup_physical_device();
    create_device_and_queuefamily();
    create_command_pool();
}

CoreInstance::~CoreInstance()
{
    cleanup();
}

// An instance is created by describing your application and any API extensions you will be using.
// After creating the instance, you can query for Vulkan supported hardware and select one or more
// VkPhysicalDevices to use for operations.
void CoreInstance::create_instance()
{

    printf("Start...");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // todo : add extension   
    
    auto ext_list = this->query_instance_extensions();      
    m_extension_list = ext_list;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(ext_list.size());
    createInfo.ppEnabledExtensionNames = ext_list.data();

    // todo : Validation layers    
    // Todo: debug utility....    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }


    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    printf("[V] Created Instance");

}

std::vector<const char*> CoreInstance::query_instance_extensions()
{
    /*
    std::vector<const char*> extension_name_list;
    //---------------------
    // Get Default vulkan extension
    //---------------------
    //To retrieve a list of supported extensions :
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    this->m_extensions.resize(extension_count); //retrieve length first
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, this->m_extensions.data());//get the list
    
    std::cout << "available extensions:" << std::endl;
    int i = 0;
    for (const auto& extension : this->m_extensions) {
        std::cout << extension.extensionName << std::endl;
        extension_name_list.emplace_back(extension.extensionName);
        i++;
    }
    //---------------------
    // Get GLFW extension
    //---------------------
    const char** glfwExtensions;
    uint32_t glfwExtensionCount = 0;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (i = 0; i < glfwExtensionCount; i++) {
        std::cout << glfwExtensions[i]<< std::endl;
        extension_name_list.emplace_back(glfwExtensions[i]);
    }
    return extension_name_list;
    */
    //---------------------
    // ToDo:...add other instance extension ( Not device extension!!)
    //---------------------      

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);        
        //extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);  // this is device extension
        
    }
    return extensions;
   
}

void CoreInstance::setup_physical_device()
{
    std::vector<VkPhysicalDevice>	m_devices;
    // query device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    m_devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, m_devices.data());

    for (const auto& device : m_devices) {
        if (is_physical_device_suitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void CoreInstance::setup_physical_property()
{
    vkGetPhysicalDeviceProperties(m_physicalDevice , &m_physical_device_properties);
}

void CoreInstance::setup_physical_features()
{
    vkGetPhysicalDeviceFeatures(m_physicalDevice , &m_physical_device_features);
}

void CoreInstance::setup_queuefamily_properties()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    m_physical_queuefamily_properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_physical_queuefamily_properties.data());
}

bool CoreInstance::is_physical_device_suitable(VkPhysicalDevice device)
{
    get_suitable_queuefamily(device , m_queueFamilyIndex);
    return m_queueFamilyIndex.is_complete() && check_device_extension_valid(device);
    
}

void CoreInstance::get_suitable_queuefamily(VkPhysicalDevice device ,QueueFamilyIndex &queue_index)
{
    //std::optional<uint32_t>                 graphic_queue_index;
    std::vector <VkQueueFamilyProperties>   physical_queuefamily_properties;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    physical_queuefamily_properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, physical_queuefamily_properties.data());

    int i = 0;
    for (const auto& queueFamily : physical_queuefamily_properties) {
        // Check if this device contains any queue family that can process graphic command.
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

            queue_index.graphic_queuefamily_index = i;            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

            if (presentSupport) {
                queue_index.present_queuefamily_index = i;
                break;
            }
        }
        i++;
    }
}

void CoreInstance::cleanup()
{
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(this->m_instance, nullptr);    
}

void CoreInstance::add_validation_layer(VkDeviceCreateInfo& createInfo)
{
    //https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Validation_layers
    if (enableValidationLayers && !check_validation_layer_valid()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    if (enableValidationLayers) {
        //std::cout << "add validation layer count " << validationLayers.size() << std::endl;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        printf("Add validation layer");
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

}

void CoreInstance::create_surface(GLFWwindow& window)
{
    if (glfwCreateWindowSurface(m_instance, &window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void CoreInstance::create_device_and_queuefamily()
{
    // The currently available drivers will only allow you to create a small number 
    // of queues for each queue family and you don't really need more than one. 
    // That's because you can create all of the command buffers on multiple threads 
    // and then submit them all at once on the main thread with a single low-overhead call.

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_queueFamilyIndex.graphic_queuefamily_index.value();
    queueCreateInfo.queueCount = 1;

    // Vulkan lets you assign priorities to queues to influence the scheduling of command 
    // buffer execution using floating point numbers between 0.0 and 1.0. This is required 
    // even if there is only a single queue:
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &m_physical_device_features;
    createInfo.enabledExtensionCount = 0; //todo: add swap-chain extension

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    add_validation_layer(createInfo);
    // the enabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo 
    // are ignored by up-to-date implementations.
    /*
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    */
  
    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    // Get the first queue from m_graphic_queuefamily and fill into vkQueue object.
    vkGetDeviceQueue(m_device, m_queueFamilyIndex.graphic_queuefamily_index.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndex.present_queuefamily_index.value(), 0, &m_presentQueue);
    

}

bool CoreInstance::check_validation_layer_valid()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

}

bool CoreInstance::check_device_extension_valid(VkPhysicalDevice device)
{
    /*
    * // From tutorial....not working
    * https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<const char*> requiredExtensions(m_extension_list.begin(), m_extension_list.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
    */
    return true;
}

void CoreInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

void CoreInstance::setupDebugMessenger()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void CoreInstance::create_command_pool()
{
    // Command buffers are executed by submitting them on one of the device queues.
   // Each command pool can only allocate command buffers that are submitted on a single type of queue. 

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    /*
    There are two possible flags for command pools:
        * VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
            Hint that command buffers are rerecorded with new commands very often
            (may change memory allocation behavior)
        * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
            Allow command buffers to be rerecorded individually, without this flag
            they all have to be reset together.
    We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
    Thus, we need to set the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
    */
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilyIndex.graphic_queuefamily_index.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

}

