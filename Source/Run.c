#include <WaterlilyRaw.h>

bool waterlily_run(waterlily_context_t *context,
                   waterlily_key_combination_t *combinations,
                   size_t combinationCount)
{
    while (waterlily_window_process(context))
    {
        if (context->window.resized)
        {
            if (!waterlily_vulkan_recreateSwapchain(context))
                return false;
            context->window.resized = false;
        }

        waterlily_input_checkKeys(context, combinations, combinationCount);

        if (!waterlily_vulkan_render(context) ||
            !waterlily_vulkan_sync(context))
            return false;
    }
    return true;
}

