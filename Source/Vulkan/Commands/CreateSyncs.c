#include <Waterlily.h>

bool waterlily_vulkan_createSyncsCommand(VkDevice device, VkFence *fences,
                                         VkSemaphore *imageAvailableSemphores,
                                         VkSemaphore *renderFinishedSemaphores)
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                                            &imageAvailableSemphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create image available semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                                   &renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create render finished semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateFence(device, &fenceInfo, nullptr, &fences[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(ERROR, "Failed to create fence %zu, code %d.",
                                 i, result);
            return false;
        }
    }
    return true;
}

