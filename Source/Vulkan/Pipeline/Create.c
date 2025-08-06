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
    pipelineInfo.pVertexInputState = &info->input;
    pipelineInfo.pInputAssemblyState = &info->assembly;
    pipelineInfo.pViewportState = &info->viewport;
    pipelineInfo.pRasterizationState = &info->rasterizer;
    pipelineInfo.pMultisampleState = &info->multisampling;
    pipelineInfo.pColorBlendState = &info->colorBlend;
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
        vkDestroyShaderModule(device, stages[1].module, nullptr);
    return true;
}

