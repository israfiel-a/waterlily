#include <Waterlily.h>
#include <stdlib.h>

bool waterlily_vulkan_createFramebuffersSwapchain(waterlily_context_t *context)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = context->pipeline.renderpass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = context->window.extent.width;
    framebufferInfo.height = context->window.extent.height;
    framebufferInfo.layers = 1;

    context->swapchain.framebuffers =
        malloc(sizeof(VkFramebuffer) * context->swapchain.imageCount);

    for (size_t i = 0; i < context->swapchain.imageCount; ++i)
    {
        framebufferInfo.pAttachments = &context->swapchain.images[i];
        VkResult result =
            vkCreateFramebuffer(context->gpu.logical, &framebufferInfo, nullptr,
                                &context->swapchain.framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create framebuffer %zu, code %d.", i, result);
            return false;
        }
        waterlily_engine_log(SUCCESS, "Created framebuffer %zu.", i);
    }
    return true;
}

