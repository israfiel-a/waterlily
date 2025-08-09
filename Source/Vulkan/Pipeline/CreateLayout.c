#include <WaterlilyRaw.h>

bool waterlily_vulkan_createLayoutPipeline(waterlily_context_t *context)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result =
        vkCreatePipelineLayout(context->gpu.logical, &pipelineLayoutInfo,
                               nullptr, &context->pipeline.layout);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create pipeline layout. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created pipeline layout.");
    return true;
}

