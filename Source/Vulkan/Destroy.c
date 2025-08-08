#include <Waterlily.h>

void waterlily_vulkan_destroy(waterlily_context_t *context)
{
    vkDestroyInstance(context->vulkan, nullptr);
}

