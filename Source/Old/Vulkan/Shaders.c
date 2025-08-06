#include <Waterlily.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

bool waterlily_compileShaders(const char **names, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        char filename[waterlily_MAX_PATH_LENGTH];
        // We only need 4, that's the length of the only file extensions being sent.
        char extension[5];
        waterlily_splitStem(names[i], filename, extension);

        waterlily_file_t file = {
            .basename = (char *)filename,
            .type = (strcmp(extension, "frag") == 0 ? waterlily_GLSL_FRAGMENT
                                                    : waterlily_GLSL_VERTEX),
        };
        if (!waterlily_glslToSPIRV(&file)) return false;
    }
    return true;
}

bool createShaderStage(const char *name, VkPipelineShaderStageCreateInfo *stage,
                       VkDevice logicalDevice)
{
    char filename[waterlily_MAX_PATH_LENGTH];
    // We only need 4, that's the length of the only file extensions being sent.
    char extension[5];
    waterlily_splitStem(name, filename, extension);

    waterlily_type_t type = strcmp(extension, "frag") == 0
                               ? waterlily_SPIRV_FRAGMENT
                               : waterlily_SPIRV_VERTEX;

    waterlily_file_t file = {
        .basename = filename,
        .type = type,
    };
    if (!waterlily_fileExists(&file) ||
        !waterlily_compileShaders((const char **)&filename, 1))
        return false;

    if (!waterlily_openFile(&file, waterlily_READ) ||
        !waterlily_getFileSize(&file))
        return false;

    char contents[file.size];
    if (!waterlily_loadFile(&file, contents) || !waterlily_closeFile(&file))
        return false;

    VkShaderModuleCreateInfo moduleCreateInfo = {0};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = file.size;
    moduleCreateInfo.pCode = (uint32_t *)contents;

    VkShaderModule module;
    VkResult result = vkCreateShaderModule(logicalDevice, &moduleCreateInfo,
                                           nullptr, &module);
    if (result != VK_SUCCESS)
    {
        waterlily_log(ERROR, "Failed to create shader module. Code: %d.",
                     result);
        return false;
    }

    *stage = (VkPipelineShaderStageCreateInfo){0};
    stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage->stage =
        (type == waterlily_SPIRV_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT
                                       : VK_SHADER_STAGE_FRAGMENT_BIT);
    stage->module = module;
    stage->pName = "main";

    return true;
}
