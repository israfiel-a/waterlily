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

    char *defaultInstanceExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface",
        "VK_KHR_get_surface_capabilities2",
        "VK_EXT_surface_maintenance1",
        "VK_EXT_debug_utils",
    };
    size_t defaultInstanceExtensionCount =
        sizeof(defaultInstanceExtensions) / sizeof(char *);

    char *defaultDeviceExtensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_swapchain_maintenance1",
    };
    size_t defaultDeviceExtensionCount =
        sizeof(defaultDeviceExtensions) / sizeof(char *);

    size_t instanceExtensionCount =
        configuration->vulkan.instanceExtensionCount +
        defaultInstanceExtensionCount;
    char *instanceExtensions[instanceExtensionCount];
    for (size_t i = 0; i < defaultInstanceExtensionCount; ++i)
        instanceExtensions[i] = defaultInstanceExtensions[i];
    for (size_t i = defaultInstanceExtensionCount; i < instanceExtensionCount;
         ++i)
        instanceExtensions[i] =
            (char *)configuration->vulkan.instanceExtensions[i];

    size_t deviceExtensionCount = configuration->vulkan.deviceExtensionCount +
                                  defaultDeviceExtensionCount;
    char *deviceExtensions[deviceExtensionCount];
    for (size_t i = 0; i < defaultDeviceExtensionCount; ++i)
        deviceExtensions[i] = defaultDeviceExtensions[i];
    for (size_t i = defaultDeviceExtensionCount; i < deviceExtensionCount; ++i)
        deviceExtensions[i] = (char *)configuration->vulkan.deviceExtensions[i];

    if (!waterlily_input_createContext(context) ||
        !waterlily_window_create(configuration->title, context) ||
        !waterlily_vulkan_create(context, (const char **)instanceExtensions,
                                 instanceExtensionCount) ||
        !waterlily_vulkan_createSurface(context) ||
        !waterlily_vulkan_getPhysicalGPU(
            context, (const char **)deviceExtensions, deviceExtensionCount) ||
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
            context, (const char **)deviceExtensions, deviceExtensionCount,
            &configuration->vulkan.deviceFeatures) ||
        !waterlily_vulkan_createSwapchain(context) ||
        !waterlily_vulkan_partitionSwapchain(context))
        return false;

    VkPipelineShaderStageCreateInfo stages[2];
    if (!waterlily_vulkan_setupShadersPipeline(context, stages))
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

