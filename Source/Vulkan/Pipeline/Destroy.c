#include <Waterlily.h>

void waterlily_vulkan_destroyPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline)
{
    vkDestroyRenderPass(device, pipeline->renderpass, nullptr);
    vkDestroyPipelineLayout(device, pipeline->layout, nullptr);
    vkDestroyPipeline(device, pipeline->pipeline, nullptr);
}

