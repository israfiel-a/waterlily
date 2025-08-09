#include <WaterlilyRaw.h>

void waterlily_vulkan_destroyGPU(waterlily_context_t *context)
{
    waterlily_vulkan_sync(context);
    vkDestroyDevice(context->gpu.logical, nullptr);
}

