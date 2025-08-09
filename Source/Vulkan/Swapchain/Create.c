#include <WaterlilyRaw.h>

bool waterlily_vulkan_createSwapchain(waterlily_context_t *context)
{
    context->swapchain.imageCount =
        context->window.capabilities.minImageCount + 1;
    if (context->window.capabilities.maxImageCount > 0 &&
        context->swapchain.imageCount >
            context->window.capabilities.maxImageCount)
        context->swapchain.imageCount =
            context->window.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context->window.surface;
    createInfo.minImageCount = context->swapchain.imageCount;
    createInfo.imageFormat = context->window.format.format;
    createInfo.imageColorSpace = context->window.format.colorSpace;
    createInfo.imageExtent = context->window.extent;
    createInfo.presentMode = context->window.mode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = context->window.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    if (context->queues.graphics.index != context->queues.present.index)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t indices[2] = {context->queues.graphics.index,
                               context->queues.present.index};
        createInfo.pQueueFamilyIndices = indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult code = vkCreateSwapchainKHR(context->gpu.logical, &createInfo,
                                         nullptr, &context->swapchain.handle);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create swapchain, code %d.",
                             code);
        return false;
    }
    return true;
}

