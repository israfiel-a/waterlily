#include <WaterlilyRaw.h>

bool waterlily_vulkan_render(waterlily_context_t *context)
{
    VkFence waitFences[] = {
        context->commandBuffers.fences[context->currentFrame],
        context->commandBuffers.presentFence};
    vkWaitForFences(context->gpu.logical, 2, waitFences, true, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        context->gpu.logical, context->swapchain.handle, UINT64_MAX,
        context->commandBuffers.imageAvailableSemphores[context->currentFrame],
        nullptr, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        return waterlily_vulkan_recreateSwapchain(context);
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        waterlily_engine_log(
            ERROR, "Failed to acquire swapchain image, code %d.", result);
        return false;
    }

    vkResetFences(context->gpu.logical, 2, waitFences);

    vkResetCommandBuffer(context->commandBuffers.buffers[context->currentFrame],
                         0);
    if (!waterlily_vulkan_recordBufferCommand(context, imageIndex))
        return false;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores =
        &context->commandBuffers.imageAvailableSemphores[context->currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &context->commandBuffers.buffers[context->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores =
        &context->commandBuffers
             .renderFinishedSemaphores[context->currentFrame];

    result =
        vkQueueSubmit(context->queues.graphics.handle, 1, &submitInfo,
                      context->commandBuffers.fences[context->currentFrame]);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to submit to the queue, code %d.",
                             result);
        return false;
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &context->commandBuffers
             .renderFinishedSemaphores[context->currentFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context->swapchain.handle;
    presentInfo.pImageIndices = &imageIndex;

    VkSwapchainPresentFenceInfoEXT presentFence = {0};
    presentFence.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT;
    presentFence.swapchainCount = 1;
    presentFence.pFences = &context->commandBuffers.presentFence;
    presentInfo.pNext = &presentFence;

    result = vkQueuePresentKHR(context->queues.graphics.handle, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        if (!waterlily_vulkan_recreateSwapchain(context))
            return false;
    }
    else if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to present swapchain image, code %d.", result);
        return false;
    }

    context->currentFrame =
        (context->currentFrame + 1) % WATERLILY_CONCURRENT_FRAMES;
    return true;
}

