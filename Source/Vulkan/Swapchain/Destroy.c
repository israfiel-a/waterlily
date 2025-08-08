#include <Waterlily.h>
#include <stdlib.h>

void waterlily_vulkan_destroySwapchain(waterlily_context_t *context)
{
    for (size_t i = 0; i < context->swapchain.imageCount; ++i)
    {
        vkDestroyFramebuffer(context->gpu.logical, context->swapchain.framebuffers[i], nullptr);
        vkDestroyImageView(context->gpu.logical, context->swapchain.images[i], nullptr);
    }
    vkDestroySwapchainKHR(context->gpu.logical, context->swapchain.handle, nullptr);
    free(context->swapchain.framebuffers);
    free(context->swapchain.images);
}

