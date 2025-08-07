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

