#include <Waterlily.h>

bool waterlily_vulkan_createLayoutPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo,
                                             nullptr, &pipeline->layout);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create pipeline layout. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created pipeline layout.");
    return true;
}

