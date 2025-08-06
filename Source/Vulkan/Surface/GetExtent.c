#include <Waterlily.h>

// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#16659263
static inline uint32_t clamp(uint32_t d, uint32_t min, uint32_t max)
{
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

void waterlily_vulkan_getExtentSurface(uint32_t width, uint32_t height,
                                       waterlily_vulkan_surface_t *surface)
{
    if (surface->capabilities.currentExtent.width != UINT32_MAX)
    {
        surface->extent = surface->capabilities.currentExtent;
        return;
    }

    surface->extent.width =
        clamp(width, surface->capabilities.minImageExtent.width,
              surface->capabilities.maxImageExtent.width);
    surface->extent.height =
        clamp(height, surface->capabilities.minImageExtent.height,
              surface->capabilities.maxImageExtent.height);
}

