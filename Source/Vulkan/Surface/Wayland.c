#include <Waterlily.h>
#include <vulkan/vulkan_wayland.h>

const char *const waterlily_vulkan_gExtensions[] = {
    "VK_KHR_surface",
    "VK_KHR_wayland_surface",
};

bool waterlily_vulkan_createSurface(waterlily_context_t *context)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = context->window.data[0];
    createInfo.surface = context->window.data[5]; 

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

