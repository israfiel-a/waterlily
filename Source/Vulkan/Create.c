#include <Waterlily.h>

bool waterlily_vulkan_create(VkInstance *instance)
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

    instanceInfo.enabledExtensionCount =
        sizeof(waterlily_vulkan_gExtensions) / sizeof(char *);
    instanceInfo.ppEnabledExtensionNames = waterlily_vulkan_gExtensions;

    const char *layers[1] = {"VK_LAYER_KHRONOS_validation"};
    instanceInfo.enabledLayerCount = 1;
    instanceInfo.ppEnabledLayerNames = layers;

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan instance. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan instance.");
    return true;
}

