#include <WaterlilyRaw.h>

// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#16659263
static inline uint32_t clamp(uint32_t d, uint32_t min, uint32_t max)
{
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

void waterlily_vulkan_getExtentSurface(waterlily_context_t *context)
{
    if (context->window.capabilities.currentExtent.width != UINT32_MAX)
    {
        context->window.extent = context->window.capabilities.currentExtent;
        return;
    }

    context->window.extent.width =
        clamp(context->window.extent.width,
              context->window.capabilities.minImageExtent.width,
              context->window.capabilities.maxImageExtent.width);
    context->window.extent.height =
        clamp(context->window.extent.height,
              context->window.capabilities.minImageExtent.height,
              context->window.capabilities.maxImageExtent.height);
}

