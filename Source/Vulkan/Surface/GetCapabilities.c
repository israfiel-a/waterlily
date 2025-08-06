#include <Waterlily.h>

bool waterlily_vulkan_getCapabilitiesSurface(
    VkPhysicalDevice device, waterlily_vulkan_surface_t *surface)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface->surface,
                                                                &surface->capabilities);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to get surface capabilities, code %d.\n", result);
        return false;
    }

    waterlily_engine_log(SUCCESS, "Got surface capabilities.");
    return true;
}

