#include <WLVulkan.h>
#include <WLWindow.h>
#include <Waterlily.h>

bool waterlily_initialize(const char *title, uint32_t version)
{
    if (!waterlily_windowCreate(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!waterlily_vulkanCompileShaders(defaults, 2)) return false;

    if (!waterlily_vulkanCreate(title, version)) return false;

    void *windowData[2];
    waterlily_windowGetData(windowData);
    if (!waterlily_vulkanCreateSurface(windowData)) return false;

    uint32_t width, height;
    waterlily_windowGetSize(&width, &height);
    if (!waterlily_vulkanInitialize(width, height)) return false;

    return true;
}

void waterlily_run(void)
{
    uint32_t width, height;
    waterlily_windowGetSize(&width, &height);
    while (waterlily_windowProcess())
    {
        if (!waterlily_vulkanRenderFrame(width, height)) return;
        if (!waterlily_vulkanSync()) return;
    }
}

void waterlily_cleanup(void) { waterlily_windowDestroy(); }
