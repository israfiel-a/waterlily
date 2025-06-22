#include <WLVulkan.h>
#include <WLWindow.h>
#include <Waterlily.h>

bool waterlily_initialize(const char *title)
{
    if (!waterlily_windowCreate(title)) return false;

    // const char *defaults[2] = {"default.vert", "default.frag"};
    // tkvul_compileShaders(defaults, 2);

    return true;
}

void waterlily_run(void)
{
    while (waterlily_windowProcess()) {}
}

void waterlily_cleanup(void) { waterlily_windowDestroy(); }
