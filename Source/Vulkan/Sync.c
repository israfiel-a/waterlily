#include <Waterlily.h>

bool waterlily_vulkan_sync(waterlily_context_t *context)
{
    VkResult result = vkDeviceWaitIdle(context->gpu.logical);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to sync, code %d.", result);
        return false;
    }
    return true;
}
