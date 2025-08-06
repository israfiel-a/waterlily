#include <Waterlily.h>

void waterlily_vulkan_destroy(VkInstance instance)
{
    vkDestroyInstance(instance, nullptr);
}

