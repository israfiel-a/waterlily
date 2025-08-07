#include <Waterlily.h>

void waterlily_vulkan_destroyBuffers(VkDevice device, VkCommandPool pool)
{
    vkDestroyCommandPool(device, pool, nullptr);
}
