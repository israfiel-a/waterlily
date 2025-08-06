#include <Waterlily.h>

bool waterlily_vulkan_partitionSwapchain(VkDevice device,
                                         waterlily_vulkan_surface_t *surface,
                                         VkSwapchainKHR swapchain,
                                         uint32_t imageCount,
                                         VkImageView *images)
{
    VkImage rawImages[imageCount];
    VkResult code =
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, rawImages);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to get swapchain images, code %d.",
                             code);
        return false;
    }

    VkImageViewCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageCreateInfo.format = surface->format.format;
    imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCreateInfo.subresourceRange.levelCount = 1;
    imageCreateInfo.subresourceRange.layerCount = 1;

    for (size_t i = 0; i < imageCount; i++)
    {
        imageCreateInfo.image = rawImages[i];
        VkResult result =
            vkCreateImageView(device, &imageCreateInfo, nullptr, &images[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create image view %zu, code %d.", i, result);
            return false;
        }
        waterlily_engine_log(SUCCESS, "Created image view %zu.", i);
    }
    return true;
}

