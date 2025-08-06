#include <Waterlily.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display *pDisplay = nullptr;
static int pScreen;
static Window pRoot;
static Window pWindow;

bool waterlily_window_create(const char *const title)
{
    pDisplay = XOpenDisplay(nullptr);
    if (pDisplay == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open display.");
        return false;
    }

    pScreen = DefaultScreen(pDisplay);
    pRoot = RootWindow(pDisplay, pScreen);
    pWindow = XCreateSimpleWindow(pDisplay, pRoot, 0, 0, 50, 50, 1,
                                  BlackPixel(pDisplay, pScreen),
                                  WhitePixel(pDisplay, pScreen));
    XSelectInput(pDisplay, pWindow, ExposureMask | KeyPressMask);
    XMapWindow(pDisplay, pWindow);

    (void)title;

    return true;
}

void waterlily_window_destroy(void)
{
    XDestroyWindow(pDisplay, pWindow);
    XCloseDisplay(pDisplay);
}

bool waterlily_window_process(void)
{
    XEvent event;
    XNextEvent(pDisplay, &event);
    switch (event.type)
    {
        case Expose:
            XFillRectangle(pDisplay, pWindow, DefaultGC(pDisplay, pScreen), 20,
                           20, 10, 10);
            break;
        case KeyPress: waterlily_window_close(WATERLILY_CLOSE_ON); break;
        default:
            waterlily_engine_log(ERROR, "Recieved unknown event '%d'.",
                                 event.type);
            break;
    }

    return true;
}

void waterlily_window_measure(uint32_t *width, uint32_t *height)
{
    *width = 50;
    *height = 50;
}

void waterlily_window_getData(void **data)
{
    data[0] = pDisplay;
    data[1] = &pWindow;
}

