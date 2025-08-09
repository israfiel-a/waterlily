#include <WaterlilyRaw.h>

bool waterlily_vulkan_recordBufferCommand(waterlily_context_t *context)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(
        context->commandBuffers.buffers[context->currentFrame], &beginInfo);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to begin command buffer, code %d.",
                             result);
        return false;
    }

    VkViewport viewport = {0};
    viewport.width = (float)context->window.extent.width;
    viewport.height = (float)context->window.extent.height;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent.width = context->window.extent.width;
    scissor.extent.height = context->window.extent.height;

    vkCmdSetViewport(context->commandBuffers.buffers[context->currentFrame], 0,
                     1, &viewport);
    vkCmdSetScissor(context->commandBuffers.buffers[context->currentFrame], 0,
                    1, &scissor);

    waterlily_vulkan_beginRenderpassCommand(context);
    vkCmdDraw(context->commandBuffers.buffers[context->currentFrame], 3, 1, 0,
              0);
    vkCmdEndRenderPass(context->commandBuffers.buffers[context->currentFrame]);

    result = vkEndCommandBuffer(
        context->commandBuffers.buffers[context->currentFrame]);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to end command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

