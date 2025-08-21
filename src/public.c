#include "internal.h"

#include <string.h>
#include <unistd.h>

waterlily_context_t context = {0};

int main(int argc, const char *const *const argv)
{
    char *workingDirectory;
    size_t workingDirectoryLength;
    if (!waterlily_engine_digest(&context, argc, argv, &workingDirectory,
                                 &workingDirectoryLength))
        return -1;
    char wd[workingDirectoryLength + 1];
    (void)strncpy(wd, workingDirectory, workingDirectoryLength);
    wd[workingDirectoryLength] = 0;

    int status = chdir(wd);
    if (status < 0)
    {
        waterlily_engine_log(ERROR,
                             "Failed to move working directory into '%s'.", wd);
        return -1;
    }
    waterlily_engine_log(SUCCESS, "Changed working directory to '%s'.", wd);

    char *deviceExtensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_swapchain_maintenance1",
    };
    size_t deviceExtensionCount = sizeof(deviceExtensions) / sizeof(char *);

    waterlily_configuration_t configuration = {0};
    if (!waterlily_engine_configure(&configuration))
        return -1;

    if (!waterlily_input_createContext(&context) ||
        !waterlily_window_create(configuration.title, &context) ||
        !waterlily_vulkan_create(&context) ||
        !waterlily_vulkan_createSurface(&context) ||
        !waterlily_vulkan_getPhysicalGPU(
            &context, (const char **)deviceExtensions, deviceExtensionCount) ||
        !waterlily_vulkan_getFormatSurface(&context) ||
        !waterlily_vulkan_getModeSurface(&context) ||
        !waterlily_vulkan_getCapabilitiesSurface(&context))
        return -1;

    waterlily_vulkan_getExtentSurface(&context);

    VkPhysicalDeviceFeatures2 requestedDeviceFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext =
            &(VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR){
                .sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
                .swapchainMaintenance1 = true,
            },
    };

    if (!waterlily_vulkan_createLogicalGPU(
            &context, (const char **)deviceExtensions, deviceExtensionCount,
            &requestedDeviceFeatures) ||
        !waterlily_vulkan_createSwapchain(&context) ||
        !waterlily_vulkan_partitionSwapchain(&context))
        return -1;

    VkPipelineShaderStageCreateInfo stages[2];
    if (!waterlily_vulkan_setupShadersPipeline(&context, stages))
        return -1;

    if (!waterlily_vulkan_createLayoutPipeline(&context) ||
        !waterlily_vulkan_createRenderpassPipeline(&context) ||
        !waterlily_vulkan_createPipeline(&context, stages, 2) ||
        !waterlily_vulkan_createFramebuffersSwapchain(&context) ||
        !waterlily_vulkan_createBuffersCommand(&context) ||
        !waterlily_vulkan_createSyncsCommand(&context))
        return -1;

    extern bool waterlily_application(waterlily_context_t * context);
    bool applicationCode = waterlily_application(&context);

    waterlily_vulkan_sync(&context);
    waterlily_vulkan_destroyBuffers(&context);
    waterlily_vulkan_destroySyncs(&context);
    waterlily_vulkan_destroySwapchain(&context);
    waterlily_vulkan_destroyPipeline(&context);
    waterlily_vulkan_destroyGPU(&context);
    waterlily_vulkan_destroySurface(&context);
    waterlily_vulkan_destroy(&context);
    waterlily_window_destroy(&context);
    waterlily_input_destroy(&context);
    return applicationCode;
}

bool waterlily_run(waterlily_context_t *context)
{
    while (waterlily_window_process(context))
    {
        if (context->window.resized)
        {
            if (!waterlily_vulkan_recreateSwapchain(context))
                return false;
            context->window.resized = false;
        }

        waterlily_input_checkKeys(context);

        if (!waterlily_vulkan_render(context))
            return false;
    }
    return true;
}

