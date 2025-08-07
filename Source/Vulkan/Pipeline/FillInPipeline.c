#include <Waterlily.h>

void waterlily_vulkan_fillInfoPipeline(waterlily_vulkan_surface_t *surface,
                                       waterlily_vulkan_pipeline_info_t *info)
{
    info->input.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    info->assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info->assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    info->viewportData.width = (float)surface->extent.width;
    info->viewportData.height = (float)surface->extent.height;
    info->viewportData.maxDepth = 1.0f;

    info->scissorData.extent = surface->extent;

    info->viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info->viewport.viewportCount = 1;
    info->viewport.pViewports = &info->viewportData;
    info->viewport.scissorCount = 1;
    info->viewport.pScissors = &info->scissorData;

    info->rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info->rasterizer.lineWidth = 1.0f;
    info->rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    info->rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    info->multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info->multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    info->colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    info->colorBlend.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info->colorBlend.attachmentCount = 1;
    info->colorBlend.pAttachments = &info->colorBlendAttachment;
}

