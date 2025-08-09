#include <WaterlilyRaw.h>

void waterlily_vulkan_destroyPipeline(waterlily_context_t *context)
{
    vkDestroyRenderPass(context->gpu.logical, context->pipeline.renderpass,
                        nullptr);
    vkDestroyPipelineLayout(context->gpu.logical, context->pipeline.layout,
                            nullptr);
    vkDestroyPipeline(context->gpu.logical, context->pipeline.handle, nullptr);
}

