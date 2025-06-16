#include <RPGtk.h>
#include <TKVulkan.h>
#include <TKWindow.h>

bool tk_initialize(void)
{
    tkwin_error_t error = tkwin_create();
    if (error != TKWIN_NO_ERROR) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    tkvul_compileShaders(defaults, 2);

    return true;
}
