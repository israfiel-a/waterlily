#include <Waterlily.h>
#include <string.h>

static uint32_t scoreDevice(VkPhysicalDevice device,
                            waterlily_vulkan_queue_indices_t *indices,
                            const VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    uint32_t score = 0;
    switch (properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 4;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 3;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score += 2;
            break;
        default:
            score += 1;
            break;
    }

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         extensions);

    size_t requiredCount =
        sizeof(waterlily_vulkan_gDeviceExtensions) / sizeof(char *);
    size_t foundCount = 0;
    for (size_t i = 0; i < extensionCount; ++i)
    {
        if (foundCount == requiredCount)
            break;

        VkExtensionProperties extension = extensions[i];
        waterlily_engine_log(INFO, "Found extension '%s'.",
                             extension.extensionName);

        for (size_t j = 0; j < requiredCount; ++j)
            if (strcmp(extension.extensionName,
                       waterlily_vulkan_gDeviceExtensions[j]) == 0)
            {
                foundCount++;
                waterlily_engine_log(SUCCESS, "Found '%s' device extension.",
                                     extension.extensionName);
                break;
            }
    }

    if (foundCount != requiredCount)
    {
        waterlily_engine_log(ERROR,
                             "Failed to find all required device extensions.");
        return 0;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    bool foundGraphicsQueue = false, foundPresentQueue = false;
    for (size_t i = 0; i < queueFamilyCount; i++)
    {
        if (foundGraphicsQueue && foundPresentQueue)
            break;

        VkQueueFamilyProperties family = queueFamilies[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices->graphics = i;
            foundGraphicsQueue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport)
        {
            indices->present = i;
            foundPresentQueue = true;
        }
    }

    if (!foundGraphicsQueue || !foundPresentQueue)
    {
        waterlily_engine_log(ERROR, "Failed to find required queue.");
        return 0;
    }
    return score;
}

bool waterlily_vulkan_getPhysicalGPU(VkInstance instance,
                                     VkPhysicalDevice *device,
                                     waterlily_vulkan_queue_indices_t *indices,
                                     const VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    VkPhysicalDevice currentChosen = nullptr;
    uint32_t bestScore = 0;
    for (size_t i = 0; i < deviceCount; i++)
    {
        uint32_t score = scoreDevice(devices[i], indices, surface);
        if (score > bestScore)
        {
            currentChosen = devices[i];
            bestScore = score;
        }
    }

    if (currentChosen == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to find suitable Vulkan device.");
        return false;
    }
    *device = currentChosen;
    waterlily_engine_log(SUCCESS, "Found suitable Vulkan device.");
    return true;
}

