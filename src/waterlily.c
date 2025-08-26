#include <internal/input.h>
#include <internal/logging.h>
#include <internal/vulkan.h>
#include <stdlib.h>
#include <unistd.h>
#include <waterlily.h>

void cleanup()
{
    waterlily_destroyVulkanContext();
    waterlily_destroyWindowContext();
}

int main(int argc, const char *const *const argv)
{
    if (atexit(cleanup) != 0)
        waterlily_report("Failed to set exit function.");

    struct waterlily_configuration *config =
        waterlily_initializeConfiguration(argc, argv);
    struct waterlily_window_context *window =
        waterlily_createWindowContext(config);
    waterlily_createVulkanContext(window);

    extern bool waterlily_application();
    if (!waterlily_application())
        return -1;

    while (waterlily_processWindowEvents())
    {
        waterlily_handleKeys();
        waterlily_renderFrame();
    }

    extern void waterlily_cleanupApplication();
    waterlily_cleanupApplication();
}

