#include <WaterlilyRaw.h>

void waterlily_vulkan_destroyBuffers(waterlily_context_t *context)
{
    vkDestroyCommandPool(context->gpu.logical, context->commandBuffers.pool,
                         nullptr);
}

