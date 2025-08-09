#include <WaterlilyRaw.h>

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
    return true;
}

