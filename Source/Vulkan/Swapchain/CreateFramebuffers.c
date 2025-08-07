#include <Waterlily.h>

bool waterlily_vulkan_createFramebuffersSwapchain(
    VkDevice device, waterlily_vulkan_surface_t *surface,
    VkRenderPass renderpass, uint32_t count, VkImageView *images,
    VkFramebuffer *framebuffers)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderpass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = surface->extent.width;
    framebufferInfo.height = surface->extent.height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < count; ++i)
    {
        framebufferInfo.pAttachments = &images[i];
        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                              &framebuffers[i]);
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

