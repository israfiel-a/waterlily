#include <Waterlily.h>

void waterlily_vulkan_beginRenderpassCommand(
    VkFramebuffer framebuffer, VkCommandBuffer buffer,
    waterlily_vulkan_surface_t *surface,
    waterlily_vulkan_graphics_pipeline_t *pipeline)
{
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pipeline->renderpass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = surface->extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->pipeline);
}

