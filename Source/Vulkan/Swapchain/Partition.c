#include <WaterlilyRaw.h>
#include <stdlib.h>

bool waterlily_vulkan_partitionSwapchain(waterlily_context_t *context)
{
    VkImage rawImages[context->swapchain.imageCount];
    VkResult code =
        vkGetSwapchainImagesKHR(context->gpu.logical, context->swapchain.handle,
                                &context->swapchain.imageCount, rawImages);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to get swapchain images, code %d.",
                             code);
        return false;
    }

    VkImageViewCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageCreateInfo.format = context->window.format.format;
    imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCreateInfo.subresourceRange.levelCount = 1;
    imageCreateInfo.subresourceRange.layerCount = 1;

    context->swapchain.images =
        malloc(sizeof(VkImageView) * context->swapchain.imageCount);
    for (size_t i = 0; i < context->swapchain.imageCount; i++)
    {
        imageCreateInfo.image = rawImages[i];
        VkResult result =
            vkCreateImageView(context->gpu.logical, &imageCreateInfo, nullptr,
                              &context->swapchain.images[i]);
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

