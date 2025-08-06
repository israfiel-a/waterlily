#include <Waterlily.h>

bool waterlily_vulkan_getModeSurface(VkPhysicalDevice device,
                                     waterlily_vulkan_surface_t *surface)
{
    uint32_t modeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface->surface, &modeCount, nullptr);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerator present modes (1), code %d.", result);
        return false;
    }

    VkPresentModeKHR modes[modeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface->surface,
                                              &modeCount, modes);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerator present modes (2), code %d.", result);
        return false;
    }

    for (size_t i = 0; i < modeCount; ++i)
    {
        // This is the best mode. If not available, we'll just select
        // VK_PRESENT_MODE_FIFO_KHR.
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            surface->mode = modes[i];
            waterlily_engine_log(SUCCESS, "Found optimal present mode.");
            return true;
        }
    }

    waterlily_engine_log(ERROR, "Failed to find optimal present mode.");
    surface->mode = VK_PRESENT_MODE_FIFO_KHR;
    return true;
}

