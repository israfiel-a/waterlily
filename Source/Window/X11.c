#include <Waterlily.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

bool waterlily_window_create(const char *const title,
                             waterlily_context_t *context)
{
    context->window.data.x11.display = XOpenDisplay(nullptr);
    if (context->window.data.x11.display == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open display.");
        return false;
    }

    context->window.data.x11.screen =
        DefaultScreen(context->window.data.x11.display);
    context->window.data.x11.root = RootWindow(context->window.data.x11.display,
                                               context->window.data.x11.screen);
    context->window.data.x11.window =
        XCreateSimpleWindow(context->window.data.x11.display,
                            context->window.data.x11.root, 0, 0, 50, 50, 1,
                            BlackPixel(context->window.data.x11.display,
                                       context->window.data.x11.screen),
                            WhitePixel(context->window.data.x11.display,
                                       context->window.data.x11.screen));
    XSelectInput(context->window.data.x11.display,
                 context->window.data.x11.window, ExposureMask | KeyPressMask);
    XMapWindow(context->window.data.x11.display,
               context->window.data.x11.window);

    (void)title;

    return true;
}

void waterlily_window_destroy(waterlily_context_t *context)
{
    XDestroyWindow(context->window.data.x11.display,
                   context->window.data.x11.window);
    XCloseDisplay(context->window.data.x11.display);
}

bool waterlily_window_process(waterlily_context_t *context)
{
    XEvent event;
    XNextEvent(context->window.data.x11.display, &event);
    switch (event.type)
    {
        case Expose:
            XFillRectangle(context->window.data.x11.display,
                           context->window.data.x11.window,
                           DefaultGC(context->window.data.x11.display,
                                     context->window.data.x11.screen),
                           20, 20, 10, 10);
            break;
        case KeyPress:
            context->window.close = true;
            break;
        default:
            waterlily_engine_log(ERROR, "Recieved unknown event '%d'.",
                                 event.type);
            break;
    }

    return true;
}

