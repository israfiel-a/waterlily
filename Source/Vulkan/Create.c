#include <WaterlilyRaw.h>

#if BUILD_TYPE == 0
static VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *data)
{
    waterlily_context_t *context = data;
    context->window.close = true;

    waterlily_engine_log(ERROR, "Vulkan error recieved: '%s'.",
                         pCallbackData->pMessage);

    return VK_FALSE;
}
#endif

bool waterlily_vulkan_create(waterlily_context_t *context,
                             const char *const *const extensions, size_t count)
{
    VkApplicationInfo applicationInfo = {0};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = nullptr;
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = nullptr;
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;

    instanceInfo.enabledExtensionCount = count;
    instanceInfo.ppEnabledExtensionNames = extensions;

#if BUILD_TYPE == 0
    constexpr size_t layerCount = 1;
    const char *const layers[layerCount] = {
        "VK_LAYER_KHRONOS_validation",
    };
#else
    constexpr size_t layerCount = 0;
    const char *const *const layers = nullptr;
#endif

    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = layers;

    VkResult result =
        vkCreateInstance(&instanceInfo, nullptr, &context->vulkan);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan instance. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan instance.");

#if BUILD_TYPE == 0
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = context;

    auto debugCreator =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->vulkan, "vkCreateDebugUtilsMessengerEXT");
    result = debugCreator(context->vulkan, &createInfo, nullptr,
                          &context->debugMessenger);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan debug messenger, code %d.", result);
        return false;
    }
#endif

    return true;
}

