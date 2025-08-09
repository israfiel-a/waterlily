#include <WaterlilyRaw.h>
#include <linux/limits.h>
#include <string.h>
#include <unistd.h>

bool waterlily_initialize(waterlily_context_t *context,
                          waterlily_configuration_t *configuration)
{
    if (!waterlily_engine_digest(context, configuration->argc,
                                 configuration->argv))
        return false;

    char wd[PATH_MAX];
    (void)strncpy(wd, context->arguments.requiredDirectory,
                  context->arguments.requiredDirectoryLength);
    wd[context->arguments.requiredDirectoryLength] = 0;

    int status = chdir(wd);
    if (status < 0)
    {
        waterlily_engine_log(ERROR,
                             "Failed to move working directory into '%s'.", wd);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Changed working directory to '%s'.", wd);

    if (!waterlily_input_createContext(context) ||
        !waterlily_window_create(configuration->title, context) ||
        !waterlily_vulkan_create(
            context, configuration->vulkan.instanceExtensions,
            configuration->vulkan.instanceExtensionCount) ||
        !waterlily_vulkan_createSurface(context) ||
        !waterlily_vulkan_getPhysicalGPU(
            context, configuration->vulkan.deviceExtensions,
            configuration->vulkan.deviceExtensionCount) ||
        !waterlily_vulkan_getFormatSurface(context) ||
        !waterlily_vulkan_getModeSurface(context) ||
        !waterlily_vulkan_getCapabilitiesSurface(context))
        return false;

    waterlily_vulkan_getExtentSurface(context);

    configuration->vulkan.deviceFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    configuration->vulkan.deviceFeatures.pNext = &(
        VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR){
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
        .pNext = nullptr,
        .swapchainMaintenance1 = true,
    };

    if (!waterlily_vulkan_createLogicalGPU(
            context, configuration->vulkan.deviceExtensions,
            configuration->vulkan.deviceExtensionCount,
            &configuration->vulkan.deviceFeatures) ||
        !waterlily_vulkan_createSwapchain(context) ||
        !waterlily_vulkan_partitionSwapchain(context))
        return false;

    VkPipelineShaderStageCreateInfo stages[2];
    const char *const stageNames[2] = {"default.vert", "default.frag"};
    if (!waterlily_vulkan_setupShadersPipeline(context, stageNames, 2, stages))
        return false;

    if (!waterlily_vulkan_createLayoutPipeline(context) ||
        !waterlily_vulkan_createRenderpassPipeline(context) ||
        !waterlily_vulkan_createPipeline(context, stages, 2,
                                         &configuration->vulkan.pipelineInfo) ||
        !waterlily_vulkan_createFramebuffersSwapchain(context) ||
        !waterlily_vulkan_createBuffersCommand(context) ||
        !waterlily_vulkan_createSyncsCommand(context))
        return false;
    return true;
}

