#include <WLVulkan.h>
#include <WLWindow.h>
#include <Waterlily.h>

bool waterlily_initialize(const char *title, uint32_t version)
{
    if (!waterlily_windowCreate(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!waterlily_vulkanCompileShaders(defaults, 2)) return false;

    if (!waterlily_vulkanCreate(title, version)) return false;

    return true;
}

void waterlily_run(void)
{
    while (waterlily_windowProcess()) {}
}

void waterlily_cleanup(void) { waterlily_windowDestroy(); }
