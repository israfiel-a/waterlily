#include <Waterlily.h>

bool waterlily_vulkan_recreateSwapchain(
    VkDevice device, waterlily_vulkan_surface_t *surface,
    waterlily_vulkan_queue_indices_t *indices,
    waterlily_vulkan_graphics_pipeline_t *pipeline, uint32_t *imageCount,
    VkFramebuffer *framebuffers, VkImageView *images, VkSwapchainKHR *swapchain)
{
    vkDeviceWaitIdle(device);

    waterlily_vulkan_destroySwapchain(device, *imageCount, framebuffers, images,
                                      *swapchain);

    uint32_t width, height;
    waterlily_window_measure(&width, &height);
    waterlily_vulkan_getExtentSurface(width, height, surface);

    if (!waterlily_vulkan_createSwapchain(device, imageCount, swapchain,
                                          surface, indices) ||
        !waterlily_vulkan_partitionSwapchain(device, surface, *swapchain,
                                             *imageCount, images) ||
        !waterlily_vulkan_createFramebuffersSwapchain(
            device, surface, pipeline->renderpass, *imageCount, images,
            framebuffers))
        return false;
    waterlily_engine_log(SUCCESS, "Recreated swapchain.");

    return true;
}

