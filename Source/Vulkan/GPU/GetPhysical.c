#include <Waterlily.h>
#include <string.h>

static uint32_t scoreDevice(waterlily_context_t *context)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(context->gpu.physical, &properties);
    vkGetPhysicalDeviceFeatures(context->gpu.physical, &features);

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
    vkEnumerateDeviceExtensionProperties(context->gpu.physical, nullptr,
                                         &extensionCount, nullptr);
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(context->gpu.physical, nullptr,
                                         &extensionCount, extensions);

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
    vkGetPhysicalDeviceQueueFamilyProperties(context->gpu.physical,
                                             &queueFamilyCount, nullptr);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(context->gpu.physical,
                                             &queueFamilyCount, queueFamilies);

    bool foundGraphicsQueue = false, foundPresentQueue = false;
    for (size_t i = 0; i < queueFamilyCount; i++)
    {
        if (foundGraphicsQueue && foundPresentQueue)
            break;

        VkQueueFamilyProperties family = queueFamilies[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            context->queues.graphics.index = i;
            foundGraphicsQueue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            context->gpu.physical, i, context->window.surface, &presentSupport);
        if (presentSupport)
        {
            context->queues.present.index = i;
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

bool waterlily_vulkan_getPhysicalGPU(waterlily_context_t *context)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->vulkan, &deviceCount, nullptr);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(context->vulkan, &deviceCount, devices);

    uint32_t bestScore = 0;
    for (size_t i = 0; i < deviceCount; i++)
    {
        context->gpu.physical = devices[i];
        uint32_t score = scoreDevice(context);
        if (score > bestScore)
            bestScore = score;
    }

    if (bestScore == 0)
    {
        waterlily_engine_log(ERROR, "Failed to find suitable Vulkan device.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Found suitable Vulkan device.");
    return true;
}

