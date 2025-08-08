#include <Waterlily.h>

void waterlily_vulkan_beginRenderpassCommand(waterlily_context_t *context)
{
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context->pipeline.renderpass;
    renderPassInfo.framebuffer =
        context->swapchain.framebuffers[context->currentFrame];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = context->window.extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(context->commandBuffers.buffers[context->currentFrame],
                         &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(context->commandBuffers.buffers[context->currentFrame],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      context->pipeline.handle);
}

