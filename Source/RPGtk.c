#include <RPGtk.h>
#include <TKVulkan.h>
#include <TKWindow.h>

bool rpgtk_initialize(const char *title)
{
    if (!rpgtk_windowCreate(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    tkvul_compileShaders(defaults, 2);

    return true;
}

void rpgtk_run(void)
{
    while (rpgtk_windowProcess()) {}
}

void rpgtk_cleanup(void) { rpgtk_windowDestroy(); }
