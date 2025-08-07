#include <Waterlily.h>

void waterlily_vulkan_destroySurface(VkInstance instance,
                                     waterlily_vulkan_surface_t *surface)
{
    vkDestroySurfaceKHR(instance, surface->surface, nullptr);
}

