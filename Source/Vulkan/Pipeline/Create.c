#include <Waterlily.h>

bool waterlily_vulkan_createPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline,
    VkPipelineShaderStageCreateInfo *stages, size_t stageCount,
    waterlily_vulkan_pipeline_info_t *info)
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = stageCount;
    pipelineInfo.pStages = stages;
    pipelineInfo.pViewportState = &(VkPipelineViewportStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    pipelineInfo.pVertexInputState = &info->input;
    pipelineInfo.pInputAssemblyState = &info->assembly;
    pipelineInfo.pRasterizationState = &info->rasterizer;
    pipelineInfo.pMultisampleState = &info->multisampling;
    pipelineInfo.pColorBlendState = &info->colorBlend;
    pipelineInfo.pDynamicState = &info->dynamic;
    pipelineInfo.layout = pipeline->layout;
    pipelineInfo.renderPass = pipeline->renderpass;

    VkResult result = vkCreateGraphicsPipelines(
        device, nullptr, 1, &pipelineInfo, nullptr, &pipeline->pipeline);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create graphics pipeline. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created graphics pipeline.");

    for (size_t i = 0; i < stageCount; ++i)
        vkDestroyShaderModule(device, stages[i].module, nullptr);
    return true;
}

