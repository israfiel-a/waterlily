#include <Waterlily.h>

bool waterlily_vulkan_getFormatSurface(waterlily_context_t *context)
{
    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->gpu.physical, context->window.surface, &formatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerate surface formats (1), code %d.", result);
        return false;
    }

    VkSurfaceFormatKHR formats[formatCount];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->gpu.physical, context->window.surface, &formatCount, formats);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerate surface formats (2), code %d.", result);
        return false;
    }

    for (size_t i = 0; i < formatCount; ++i)
    {
        VkSurfaceFormatKHR currentFormat = formats[i];
        // This is the best combination. If not available, we'll just
        // select the first provided colorspace.
        if (currentFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            context->window.format = currentFormat;
            waterlily_engine_log(SUCCESS, "Found optimal surface format.");
            return true;
        }
    }

    waterlily_engine_log(
        ERROR, "Could not find the optimal colorspace. Falling back on %dx%d.",
        formats[0].format, formats[0].colorSpace);
    context->window.format = formats[0];
    return true;
}

