#include <Waterlily.h>
#include <string.h>

bool waterlily_vulkan_setupShadersPipeline(
    VkDevice device, const char *const *const stages, size_t count,
    VkPipelineShaderStageCreateInfo *storage)
{
    char *argv[] = {
        "glslang",   "-e",      "main",
#if BUILD_TYPE == 0
        "-g",
#else
        "-g0",
#endif
        "-o",        nullptr,   "-t",          "--glsl-version",
        "460",       "--quiet", "--spirv-val", "--target-env",
        "vulkan1.3", "--lto",   nullptr,       nullptr,
    };

    for (size_t i = 0; i < count; ++i)
    {
        const char *const file = stages[i];
        size_t fileLength = strlen(file) + 25;
        char fullPath[fileLength], outputPath[fileLength + 2];
        snprintf(fullPath, fileLength, "Assets/Shaders/Source/%s", file);
        snprintf(outputPath, fileLength, "Assets/Shaders/Compiled/%s", file);
        argv[5] = outputPath;
        argv[14] = fullPath;
        if (!waterliliy_files_execute(argv))
            return false;

        FILE *output;
        if (!waterlily_files_open(outputPath, &output))
            return false;

        size_t fileSize;
        if (!waterlily_files_measure(output, &fileSize))
            return false;
        char fileContents[fileSize];
        if (!waterlily_files_read(output, fileSize, (uint8_t *)fileContents))
            return false;
        waterlily_files_close(output);

        VkShaderModuleCreateInfo moduleCreateInfo = {0};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = fileSize;
        moduleCreateInfo.pCode = (uint32_t *)fileContents;

        VkShaderModule module;
        VkResult result =
            vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create shader module. Code: %d.", result);
            return false;
        }

        VkPipelineShaderStageCreateInfo *stage = &storage[i];
        stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage->stage = (strcmp(strrchr(file, '.') + 1, "vert") == 0
                            ? VK_SHADER_STAGE_VERTEX_BIT
                            : VK_SHADER_STAGE_FRAGMENT_BIT);
        stage->module = module;
        stage->pName = "main";
    }

    return true;
}

