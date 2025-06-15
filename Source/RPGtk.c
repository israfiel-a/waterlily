#include <RPGtk.h>
#include <TKWindow.h>

bool tk_initialize(void)
{
    tkwin_error_t error = tkwin_create();
    if (error != TKWIN_NO_ERROR) return false;

    return true;
}
