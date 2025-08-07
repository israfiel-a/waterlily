#include <Waterlily.h>

bool waterlily_vulkan_recordBufferCommand(
    VkCommandBuffer commandBuffer, waterlily_vulkan_surface_t *surface,
    waterlily_vulkan_graphics_pipeline_t *pipeline, VkFramebuffer framebuffer)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to begin command buffer, code %d.",
                             result);
        return false;
    }

    VkViewport viewport = {0};
    uint32_t width, height;
    waterlily_window_measure(&width, &height);
    viewport.width = (float)width;
    viewport.height = (float)height;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent.width = width;
    scissor.extent.height = height;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    waterlily_vulkan_beginRenderpassCommand(framebuffer, commandBuffer, surface,
                                            pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to end command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

