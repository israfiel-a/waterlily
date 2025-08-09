#include <WaterlilyRaw.h>

bool waterlily_vulkan_createBuffersCommand(waterlily_context_t *context)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = context->queues.graphics.index;

    VkResult result =
        vkCreateCommandPool(context->gpu.logical, &poolInfo, nullptr,
                            &context->commandBuffers.pool);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command pool, code %d.",
                             result);
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context->commandBuffers.pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = WATERLILY_CONCURRENT_FRAMES;

    result = vkAllocateCommandBuffers(context->gpu.logical, &allocInfo,
                                      context->commandBuffers.buffers);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

