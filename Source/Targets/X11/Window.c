#include <Waterlily.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/**
 * @var bool pClose
 * @brief The global close variable, which is assigned in order to, well, close
 * the window. This does @b not instantly kill the window, it simply gives a
 * gentle nudge to begin resource deaquisition.
 * @since v0.0.0.20
 */
bool pClose = false;
static Display* pDisplay = nullptr;
static int pScreen;
static Window pRoot;
static Window pWindow;

bool waterlily_createWindow(const char* const title) {
    pDisplay = XOpenDisplay(nullptr);
    if(pDisplay == nullptr) {
        waterlily_log(ERROR, "Failed to open display.");
        return false;
    }

    pScreen = DefaultScreen(pDisplay);
    pRoot = RootWindow(pDisplay, pScreen);
    pWindow = XCreateSimpleWindow(pDisplay, pRoot, 0, 0, 50, 50, 1, BlackPixel(pDisplay, pScreen), WhitePixel(pDisplay, pScreen));
    XSelectInput(pDisplay, pWindow, ExposureMask | KeyPressMask);
    XMapWindow(pDisplay, pWindow);

    (void)title;

    return true; 
}

void waterlily_destroyWindow(void) {
    XDestroyWindow(pDisplay, pWindow);
    XCloseDisplay(pDisplay);
}

bool waterlily_processWindow(void) { 
    XEvent event;
    XNextEvent(pDisplay, &event);
    switch(event.type) {
        case Expose:
            XFillRectangle(pDisplay, pWindow, DefaultGC(pDisplay, pScreen), 20, 20, 10, 10);
            break;
        case KeyPress:
            pClose = true;
            break;
        default:
            waterlily_log(ERROR, "Recieved unknown event '%d'.", event.type);
            break;
    }

    return true; 
}

void waterlily_getSizeWindow(uint32_t *width, uint32_t *height) { *width = 50; *height = 50; }

void waterlily_getDataWindow(void **data) { data[0] = pDisplay; data[1] = &pWindow; }


