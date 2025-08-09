#include <WaterlilyRaw.h>

bool waterlily_vulkan_createLogicalGPU(waterlily_context_t *context,
                                       const char *const *const extensions,
                                       size_t count,
                                       VkPhysicalDeviceFeatures2 *features)
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {{0}, {0}};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = context->queues.graphics.index;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &priority;

    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = context->queues.present.index;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &priority;

    // Layers for logical devices no longer need to be set in newer
    // implementations.
    VkDeviceCreateInfo logicalDeviceCreateInfo = {0};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    logicalDeviceCreateInfo.queueCreateInfoCount =
        context->queues.graphics.index == context->queues.present.index ? 1 : 2;
    logicalDeviceCreateInfo.pEnabledFeatures = nullptr;
    logicalDeviceCreateInfo.pNext = features;

    logicalDeviceCreateInfo.enabledExtensionCount = count;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = extensions;

    VkResult code =
        vkCreateDevice(context->gpu.physical, &logicalDeviceCreateInfo, nullptr,
                       &context->gpu.logical);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create logical device. Code: %d.", code);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created logical device.");

    vkGetDeviceQueue(context->gpu.logical, context->queues.graphics.index, 0,
                     &context->queues.graphics.handle);
    vkGetDeviceQueue(context->gpu.logical, context->queues.present.index, 0,
                     &context->queues.present.handle);
    waterlily_engine_log(INFO, "Created device data queues.");
    return true;
}

