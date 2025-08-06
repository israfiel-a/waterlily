#include <Waterlily.h>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

const char *const waterlily_vulkan_gExtension[2] = {
    "VK_KHR_surface",
    "VK_KHR_xlib_surface",
};

bool waterlily_vulkan_createSurface(VkInstance instance,
                                    waterlily_vulkan_surface_t *surface)
{
    VkXlibSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;

    void *data[2];
    waterlily_window_getData(data);
    createInfo.dpy = data[0];
    createInfo.window = *(Window *)data[1];

    VkResult code = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr,
                                           &surface->surface);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan-X11 interop surface. Code: %d.",
            code);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan-X11 interop surface.");
    return true;
}

