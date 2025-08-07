#include <Waterlily.h>

bool waterlily_vulkan_sync(VkDevice device)
{
    VkResult result = vkDeviceWaitIdle(device);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to sync, code %d.", result);
        return false;
    }
    return true;
}
