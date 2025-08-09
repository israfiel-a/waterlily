#include <WaterlilyRaw.h>

void waterlily_vulkan_destroy(waterlily_context_t *context)
{
#if BUILD_TYPE == 0
    auto debugDestroy =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->vulkan, "vkDestroyDebugUtilsMessengerEXT");
    debugDestroy(context->vulkan, context->debugMessenger, nullptr);
#endif

    vkDestroyInstance(context->vulkan, nullptr);
}

