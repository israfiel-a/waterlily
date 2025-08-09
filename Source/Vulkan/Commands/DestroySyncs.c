#include <WaterlilyRaw.h>

void waterlily_vulkan_destroySyncs(waterlily_context_t *context)
{
    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        vkDestroySemaphore(context->gpu.logical,
                           context->commandBuffers.imageAvailableSemphores[i],
                           nullptr);
        vkDestroySemaphore(context->gpu.logical,
                           context->commandBuffers.renderFinishedSemaphores[i],
                           nullptr);
        vkDestroyFence(context->gpu.logical, context->commandBuffers.fences[i],
                       nullptr);
    }
}

