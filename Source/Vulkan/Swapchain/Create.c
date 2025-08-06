#include <Waterlily.h>

bool waterlily_vulkan_createSwapchain(VkDevice device, uint32_t *imageCount,
                                      VkSwapchainKHR *swapchain,
                                      VkSurfaceKHR surface,
                                      waterlily_vulkan_queue_indices_t *indices)
{
    VkSurfaceFormatKHR format = getSurfaceFormat(pPhysicalDevice);
    VkPresentModeKHR mode = getSurfaceMode(pPhysicalDevice);
    VkSurfaceCapabilitiesKHR capabilities =
        getSurfaceCapabilities(pPhysicalDevice);

    *imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        *imageCount > capabilities.maxImageCount)
        *imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = *imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = *extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = mode;
    createInfo.clipped = VK_TRUE;
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

