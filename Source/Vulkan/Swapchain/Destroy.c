#include <Waterlily.h>

void waterlily_vulkan_destroySwapchain(VkDevice device, uint32_t imageCount,
                                       VkFramebuffer *framebuffers,
                                       VkImageView *images,
                                       VkSwapchainKHR swapchain)
{
    for (size_t i = 0; i < imageCount; ++i)
    {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, images[i], nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

