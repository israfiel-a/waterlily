#include <WaterlilyRaw.h>

void waterlily_deinitialize(waterlily_context_t *context)
{
    waterlily_vulkan_sync(context);
    waterlily_vulkan_destroyBuffers(context);
    waterlily_vulkan_destroySyncs(context);
    waterlily_vulkan_destroySwapchain(context);
    waterlily_vulkan_destroyPipeline(context);
    waterlily_vulkan_destroyGPU(context);
    waterlily_vulkan_destroySurface(context);
    waterlily_vulkan_destroy(context);
    waterlily_window_destroy(context);
    waterlily_input_destroy(context);
}

