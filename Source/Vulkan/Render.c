#include <Waterlily.h>

bool waterlily_vulkan_render(VkDevice device,
                             waterlily_vulkan_surface_t *surface,
                             waterlily_vulkan_queue_indices_t *indices,
                             waterlily_vulkan_queues_t *queues,
                             waterlily_vulkan_graphics_pipeline_t *pipeline,
                             VkCommandBuffer buffer, VkFence fence,
                             VkSemaphore imageAvailableSemaphore,
                             VkSemaphore renderFinishedSemaphore,
                             VkSwapchainKHR *swapchain, uint32_t *imageCount,
                             VkFramebuffer *framebuffers, VkImageView *images)
{
    vkWaitForFences(device, 1, &fence, true, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result =
        vkAcquireNextImageKHR(device, *swapchain, UINT64_MAX,
                              imageAvailableSemaphore, nullptr, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        return waterlily_vulkan_recreateSwapchain(
            device, surface, indices, pipeline, imageCount, framebuffers,
            images, swapchain);
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        waterlily_engine_log(
            ERROR, "Failed to acquire swapchain image, code %d.", result);
        return false;
    }

    vkResetFences(device, 1, &fence);

    vkResetCommandBuffer(buffer, 0);
    if (!waterlily_vulkan_recordBufferCommand(buffer, surface, pipeline,
                                              framebuffers[imageIndex]))
        return false;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
    result = vkQueueSubmit(queues->graphics, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to submit to the queue, code %d.",
                             result);
        return false;
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(queues->present, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        if (!waterlily_vulkan_recreateSwapchain(
                device, surface, indices, pipeline, imageCount, framebuffers,
                images, swapchain))
            return false;
    }
    else if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to present swapchain image, code %d.", result);
        return false;
    }

    return true;
}

