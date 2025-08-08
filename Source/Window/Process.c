#include <Waterlily.h>

bool waterlily_window_process(waterlily_context_t *context)
{
    return wl_display_dispatch(context->window.data.display) != -1 &&
           !context->window.close;
}

