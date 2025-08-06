#include <Waterlily.h>

void waterlily_vulkan_destroyGPU(VkDevice logical)
{
    vkDeviceWaitIdle(logical);
    vkDestroyDevice(logical, nullptr); 
}

