#include <Waterlily.h>

bool waterlily_window_resized(waterlily_resize_type_t type)
{
    static bool resized = false;
    if (type == WATERLILY_RESIZE_YES)
        resized = true;
    else if (type == WATERLILY_RESIZE_NO)
        resized = false;

    return resized;
}

