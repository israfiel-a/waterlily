#include <WaterlilyRaw.h>

bool waterlily_vulkan_getCapabilitiesSurface(waterlily_context_t *context)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context->gpu.physical, context->window.surface,
        &context->window.capabilities);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to get surface capabilities, code %d.\n", result);
        return false;
    }

    waterlily_engine_log(SUCCESS, "Got surface capabilities.");
    return true;
}

