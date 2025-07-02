#include <Geranium.h>
#include <Hyacinth.h>
#define PRIMROSE_IMPLEMENTATION
#include <Primrose.h>
#include <Waterlily.h>

bool waterlily_initialize(const char *title, uint32_t version)
{
    if (!hyacinth_create(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!geranium_compileShaders(defaults, 2)) return false;

    if (!geranium_create(title, version)) return false;

    primrose_log(SUCCESS, "Initialized engine.");
    return true;
}

void waterlily_run(void)
{
    uint32_t width, height;
    hyacinth_getSize(&width, &height);
    while (hyacinth_process())
    {
        if (!geranium_render(width, height)) return;
        if (!geranium_sync()) return;
    }
}

void waterlily_cleanup(void)
{
    geranium_destroy();
    hyacinth_destroy();
    primrose_log(SUCCESS, "Cleaned up engine.");
}
