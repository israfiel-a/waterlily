#include <RPGtk.h>
#include <TKVulkan.h>
#include <TKWindow.h>

bool tk_initialize(const char *title)
{
    if (!rpgtk_windowCreate(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    tkvul_compileShaders(defaults, 2);

    return true;
}
