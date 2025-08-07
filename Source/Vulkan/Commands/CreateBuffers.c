#include <Waterlily.h>

bool waterlily_vulkan_createBuffersCommand(
    VkDevice device, waterlily_vulkan_queue_indices_t *indices,
    VkCommandPool *commandPool, VkCommandBuffer *buffers)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices->graphics;

    VkResult result =
        vkCreateCommandPool(device, &poolInfo, nullptr, commandPool);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command pool, code %d.",
                             result);
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = WATERLILY_CONCURRENT_FRAMES;

    result = vkAllocateCommandBuffers(device, &allocInfo, buffers);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

