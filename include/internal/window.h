#ifndef WATERLILY_INTERNAL_WINDOW_H
#define WATERLILY_INTERNAL_WINDOW_H

#include "config.h"

#include <stdint.h>
#include <wayland-client.h>

struct waterlily_window_context
{
    uint32_t scale;
    bool resized;
    bool close;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_output *output;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;
    struct wl_surface *surface;
    void *shell;
    void *shellSurface;
    void *toplevel;
    uint32_t width;
    uint32_t height;
};

struct waterlily_window_context *
waterlily_createWindowContext(struct waterlily_configuration *config);
void waterlily_destroyWindowContext(void);
bool waterlily_processWindowEvents(void);

#endif // WATERLILY_INTERNAL_WINDOW_H

