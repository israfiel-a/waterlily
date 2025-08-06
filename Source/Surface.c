#include <Waterlily.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>

VkSurfaceKHR gSurface = nullptr;

static uint32_t pFormatCount = 0;
static VkSurfaceFormatKHR *pFormats = nullptr;
static VkPresentModeKHR *pModes = nullptr;
static uint32_t pModeCount = 0;

static VkSurfaceCapabilitiesKHR pCapabilities;
static VkSurfaceFormatKHR pFormat;
static VkPresentModeKHR pMode;

// TODO: Get this the fuck outta here.
// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#16659263
uint32_t _clamp(uint32_t d, uint32_t min, uint32_t max)
{
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

VkExtent2D getSurfaceExtent(uint32_t width, uint32_t height)
{
    if (pCapabilities.currentExtent.width != UINT32_MAX)
        return pCapabilities.currentExtent;

    VkExtent2D surfaceExtent = {.width = width, .height = height};

    surfaceExtent.width =
        _clamp(surfaceExtent.width, pCapabilities.minImageExtent.width,
               pCapabilities.maxImageExtent.width);
    surfaceExtent.height =
        _clamp(surfaceExtent.height, pCapabilities.minImageExtent.height,
               pCapabilities.maxImageExtent.height);

    return surfaceExtent;
}

VkSurfaceFormatKHR getSurfaceFormat([[maybe_unused]] VkPhysicalDevice device)
{
    if(__builtin_expect(pFormats != nullptr, 1)) {
        for (size_t i = 0; i < pFormatCount; i++)
        {
            VkSurfaceFormatKHR format = pFormats[i];
            // This is the best combination. If not available, we'll just
            // select the first provided colorspace.
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                pFormat = format;
                return pFormat;
            }
        }
        return pFormats[0];
    }

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, gSurface, &pFormatCount, nullptr);
    pFormats = malloc(sizeof(VkSurfaceFormatKHR) * pFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, gSurface, &pFormatCount,
                                         pFormats);
    return getSurfaceFormat(device);
}

VkPresentModeKHR getSurfaceMode([[maybe_unused]] VkPhysicalDevice device)
{
    if (__builtin_expect(pModes != nullptr, 1)) {
        for (size_t i = 0; i < pModeCount; i++)
        {
            // This is the best mode. If not available, we'll just select
            // VK_PRESENT_MODE_FIFO_KHR.
            if (pModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                pMode = pModes[i];
                return pMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, gSurface, &pModeCount,
                                              nullptr);
    pModes = malloc(sizeof(VkPresentModeKHR) * pModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, gSurface, &pModeCount,
                                              pModes);
    return getSurfaceMode(device);
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities([[maybe_unused]] VkPhysicalDevice device) {
    if(__builtin_expect(pCapabilities.maxImageCount == 0, 0)) 
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, gSurface, &pCapabilities);
    return pCapabilities; 
}

