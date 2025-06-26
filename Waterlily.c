#include <Geranium.h>
#include <Hyacinth.h>
#define PRIMROSE_IMPLEMENTATION
#include <Primrose.h>
#include <Waterlily.h>

bool waterlily_initialize(const char *title, uint32_t version)
{
    if (!hyacinth_create(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!waterlily_vulkanCompileShaders(defaults, 2)) return false;

    if (!waterlily_vulkanCreate(title, version)) return false;

    void *windowData[2];
    hyacinth_getData(windowData);
    if (!waterlily_vulkanCreateSurface(windowData)) return false;

    uint32_t width, height;
    hyacinth_getSize(&width, &height);
    if (!waterlily_vulkanInitialize(width, height)) return false;

    primrose_log(SUCCESS, "Initialized engine.");
    return true;
}

void waterlily_run(void)
{
    uint32_t width, height;
    hyacinth_getSize(&width, &height);
    while (hyacinth_process())
    {
        if (!waterlily_vulkanRenderFrame(width, height)) return;
        if (!waterlily_vulkanSync()) return;
    }
}

void waterlily_cleanup(void)
{
    hyacinth_destroy();
    primrose_log(SUCCESS, "Cleaned up engine.");
}
