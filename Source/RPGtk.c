#include <RPGtk.h>
#include <TKVulkan.h>
#include <TKWindow.h>

bool tk_initialize(void)
{
    if (!tkwin_create()) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    tkvul_compileShaders(defaults, 2);

    return true;
}
