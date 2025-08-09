#include <WaterlilyRaw.h>

void waterlily_vulkan_destroySurface(waterlily_context_t *context)
{
    vkDestroySurfaceKHR(context->vulkan, context->window.surface, nullptr);
}

