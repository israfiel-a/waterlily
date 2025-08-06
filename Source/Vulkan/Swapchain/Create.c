#include <Waterlily.h>

bool waterlily_vulkan_createSwapchain(VkDevice device, uint32_t *imageCount,
                                      VkSwapchainKHR *swapchain,
                                      waterlily_vulkan_surface_t *surface,
                                      waterlily_vulkan_queue_indices_t *indices)
{
    *imageCount = surface->capabilities.minImageCount + 1;
    if (surface->capabilities.maxImageCount > 0 &&
        *imageCount > surface->capabilities.maxImageCount)
        *imageCount = surface->capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;
    createInfo.minImageCount = *imageCount;
    createInfo.imageFormat = surface->format.format;
    createInfo.imageColorSpace = surface->format.colorSpace;
    createInfo.imageExtent = surface->extent;
    createInfo.presentMode = surface->mode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = surface->capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    if (indices->graphics != indices->present)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = (uint32_t *)indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult code =
        vkCreateSwapchainKHR(device, &createInfo, nullptr, swapchain);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create swapchain, code %d.",
                             code);
        return false;
    }
    return true;
}

