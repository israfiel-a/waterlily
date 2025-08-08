#include <Waterlily.h>

bool waterlily_vulkan_recreateSwapchain(waterlily_context_t *context)
{
    waterlily_vulkan_sync(context);
    waterlily_vulkan_destroySwapchain(context);

    if (!waterlily_vulkan_createSwapchain(context) ||
        !waterlily_vulkan_partitionSwapchain(context) ||
        !waterlily_vulkan_createFramebuffersSwapchain(context))
        return false;
    waterlily_engine_log(SUCCESS, "Recreated swapchain.");

    return true;
}

