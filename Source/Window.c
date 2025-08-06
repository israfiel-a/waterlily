#include <Waterlily.h>

void waterlily_closeWindow(void)
{
    extern bool gClose;
    gClose = true;

    waterlily_log(INFO, "Closed window.");
}

