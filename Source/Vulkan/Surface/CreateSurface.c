#include <WaterlilyRaw.h>
#include <vulkan/vulkan_wayland.h>

bool waterlily_vulkan_createSurface(waterlily_context_t *context)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = context->window.data.display;
    createInfo.surface = context->window.data.surface; 

    VkResult code = vkCreateWaylandSurfaceKHR(
        context->vulkan, &createInfo, nullptr, &context->window.surface);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan-Wayland interop surface. Code: %d.",
            code);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan-Wayland interop surface.");
    return true;
}

