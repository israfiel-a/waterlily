#include <WaterlilyRaw.h>

bool waterlily_vulkan_createSyncsCommand(waterlily_context_t *context)
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        VkResult result = vkCreateSemaphore(
            context->gpu.logical, &semaphoreInfo, nullptr,
            &context->commandBuffers.imageAvailableSemphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create image available semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateSemaphore(
            context->gpu.logical, &semaphoreInfo, nullptr,
            &context->commandBuffers.renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create render finished semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateFence(context->gpu.logical, &fenceInfo, nullptr,
                               &context->commandBuffers.fences[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(ERROR, "Failed to create fence %zu, code %d.",
                                 i, result);
            return false;
        }
    }
    return true;
}

