#include <Waterlily.h>
#include <vulkan/vulkan_wayland.h>

const char *const waterlily_vulkan_gExtensions[] = {
    "VK_KHR_surface",
    "VK_KHR_wayland_surface",
};

bool waterlily_vulkan_createSurface(VkInstance instance,
                                    waterlily_vulkan_surface_t *surface)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;

    void *data[2];
    waterlily_window_getData(data);
    createInfo.display = data[0];
    createInfo.surface = data[1];

    VkResult code = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr,
                                              &surface->surface);
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

