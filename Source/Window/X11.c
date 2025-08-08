#include <Waterlily.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>

// fix this via union instead of void *data you fucking idiot
static unsigned long window;

bool waterlily_window_create(const char *const title,
                             waterlily_context_t *context)
{
    context->window.data = malloc(sizeof(void *) * 2);
    context->window.data[0] = XOpenDisplay(nullptr);
    if (context->window.data[0] == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open display.");
        return false;
    }

    int screen = DefaultScreen(context->window.data[0]);
    int root = RootWindow(context->window.data[0], screen);
    window = XCreateSimpleWindow(context->window.data[0], root, 0, 0, 50, 50, 1,
                                 BlackPixel(context->window.data[0], screen),
                                 WhitePixel(context->window.data[0], screen));
    context->window.data[1] = &window;
    XSelectInput(context->window.data[0], (Window)context->window.data[1],
                 ExposureMask | KeyPressMask);
    XMapWindow(context->window.data[0], window);

    (void)title;

    return true;
}

void waterlily_window_destroy(waterlily_context_t *context)
{
    XDestroyWindow(context->window.data[0], window);
    XCloseDisplay(context->window.data[0]);
    free(context->window.data);
}

bool waterlily_window_process(waterlily_context_t *context)
{
    XEvent event;
    XNextEvent(context->window.data[0], &event);
    switch (event.type)
    {
        case Expose:
            XFillRectangle(context->window.data[0], window,
                           DefaultGC(context->window.data[0],
                                     DefaultScreen(context->window.data[0])),
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

