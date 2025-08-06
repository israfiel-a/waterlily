#include <Waterlily.h>

bool waterlily_vulkan_createLogicalGPU(
    const VkPhysicalDevice physical, VkDevice *device,
    waterlily_vulkan_queues_t *queues,
    const waterlily_vulkan_queue_indices_t *indices)
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {{0}, {0}};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = indices->graphics;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &priority;

    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = indices->present;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &priority;

    // We don't need any features at the moment.
    VkPhysicalDeviceFeatures usedFeatures = {0};

    // Layers for logical devices no longer need to be set in newer
    // implementations.
    VkDeviceCreateInfo logicalDeviceCreateInfo = {0};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    logicalDeviceCreateInfo.queueCreateInfoCount =
        indices->graphics == indices->present ? 1 : 2;
    logicalDeviceCreateInfo.pEnabledFeatures = &usedFeatures;

    logicalDeviceCreateInfo.enabledExtensionCount =
        sizeof(waterlily_vulkan_gDeviceExtensions) / sizeof(char *);
    logicalDeviceCreateInfo.ppEnabledExtensionNames =
        waterlily_vulkan_gDeviceExtensions;

    VkResult code =
        vkCreateDevice(physical, &logicalDeviceCreateInfo, nullptr, device);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create logical device. Code: %d.", code);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created logical device.");

    vkGetDeviceQueue(*device, indices->graphics, 0, &queues->graphics);
    vkGetDeviceQueue(*device, indices->present, 0, &queues->present);
    waterlily_engine_log(INFO, "Created device data queues.");
    return true;
}

