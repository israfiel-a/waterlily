#include <WaterlilyRaw.h>
#include <string.h>

static bool createModule(VkDevice device, const char *const filename,
                         VkPipelineShaderStageCreateInfo *stage)
{
    FILE *output;
    if (!waterlily_files_open(filename, &output))
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
        waterlily_engine_log(ERROR, "Failed to create shader module. Code: %d.",
                             result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created shader module.");

    stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage->stage = (strcmp(strrchr(filename, '.') + 1, "vert") == 0
                        ? VK_SHADER_STAGE_VERTEX_BIT
                        : VK_SHADER_STAGE_FRAGMENT_BIT);
    stage->module = module;
    stage->pName = "main";
    return true;
}

bool waterlily_vulkan_setupShadersPipeline(
    waterlily_context_t *context, const char *const *const stages, size_t count,
    VkPipelineShaderStageCreateInfo *storage)
{
    char *argv[] = {
        "/usr/bin/glslang",
        "-e",
        "main",
#if BUILD_TYPE == 0
        "-g",
#else
        "-g0",
#endif
        "-o",
        nullptr,
        "-t",
        "--glsl-version",
        "460",
        "--quiet",
        "--spirv-val",
        "--target-env",
        "vulkan1.3",
        "--lto",
        nullptr,
        nullptr,
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

        if (!waterlily_files_exists(fullPath))
        {
            // Already did the compilation in a prior run-through.
            if (waterlily_files_exists(outputPath))
            {
                if (!createModule(context->gpu.logical, outputPath,
                                  &storage[i]))
                    return false;
                continue;
            }

            waterlily_engine_log(ERROR, "Failed to find provided shader '%s'.",
                                 fullPath);
            return false;
        }

        if (!waterlily_files_execute(argv))
            return false;
        waterlily_engine_log(SUCCESS, "Compiled shader '%s'.", fullPath);

        if (!createModule(context->gpu.logical, outputPath, &storage[i]))
            return false;
    }

    if (!waterlily_files_exists("Assets/Shaders/Source/"))
    {
        waterlily_engine_log(WARNING, "Shader source directory missing.");
        return true;
    }

    if (!waterlily_files_remove("Assets/Shaders/Source/"))
    {
        waterlily_engine_log(ERROR,
                             "Failed to remove shader source directory.");
        return false;
    }
    return true;
}

