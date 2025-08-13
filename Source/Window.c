/**
 * @brief This file provides the complete implementation of the
 * waterlily interface. This only depends upon the default C-standard @c
 * stdint.h, and @c string.h files, and the client header @c
 *-client.h. I hate this interface. I think it's awful, and it
 * introduces data that can only be properly handled as private, global
 * variables. Yuck.
 * @since v0.0.1
 *
 * @note This file contains material (the contents of the XDG-shell protocol)
 * copyrighted by the following people. All rights are reserved to their proper
 * owners.
 * Copyright © 2008-2013 Kristian Høgsberg
 * Copyright © 2013      Rafael Antognolli
 * Copyright © 2013      Jasper St. Pierre
 * Copyright © 2010-2013 Intel Corporation
 * Copyright © 2015-2017 Samsung Electronics Co., Ltd
 * Copyright © 2015-2017 Red Hat Inc.
 *
 * @copyright (C) 2025 - Israfil Argos
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details. You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "Internal.h"

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * @brief Convert an interface into a double-referenced pointer via some casting
 * nonsense. This is for the sole purpose of creating other interfaces (whose
 * function signatures require an "array" of interfaces) and should not be used
 * outside of this purpose. For all intents and purposes, this macro does not
 * exist; ignore it.
 * @since v0.0.1
 *
 * @param[in] interface The interface to convert into a double-referenced
 * pointer, or "array".
 * @return A constant @c wl_interface double-pointer.
 */
#define REFREF(interface) (const struct wl_interface **)(&interface)

/**
 * @brief The XDG toplevel interface, including functions that can be called and
 * events that can be applied. This is the version seven interface.
 * @since v0.0.1
 *
 * @remark Most of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface pXDGToplevelInterface = {
    .name = "xdg_toplevel",
    .version = 7,
    .method_count = 14,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {0},
            {"set_title", "s", nullptr},
            {"set_app_id", "s", nullptr},
            {0},
            {0},
            {0},
            {0},
            {0},
            {0},
            {0},
            {"set_fullscreen", "?o", REFREF(wl_output_interface)},
            {0},
            {0},
        },
    .event_count = 4,
    .events =
        (struct wl_message[]){
            {"configure", "iia", nullptr},
            {"close", "", nullptr},
            {"configure_bounds", "4ii", nullptr},
            {"wm_capabilities", "5a", nullptr},
        },
};

/**
 * @brief The XDG surface interface, including functions that can be called and
 * events that can be applied. This is the version seven interface.
 * @since v0.0.1
 *
 * @remark Some of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface pXDGSurfaceInterface = {
    .name = "xdg_surface",
    .version = 7,
    .method_count = 5,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {"get_toplevel", "n", REFREF(pXDGToplevelInterface)},
            {0},
            {0},
            {"ack_configure", "u", nullptr},
        },
    .event_count = 1,
    .events = (struct wl_message[]){{"configure", "u", nullptr}},
};

/**
 * @brief The XDG window manager base interface, including functions that can be
 * called and events that can be applied. This is the version seven interface.
 * @since v0.0.1
 *
 * @remark One of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface pXDGShellInterface = {
    .name = "xdg_wm_base",
    .version = 7,
    .method_count = 4,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {0},
            {"get_xdg_surface", "no", REFREF(pXDGSurfaceInterface)},
            {"pong", "u", nullptr},
        },
    .event_count = 1,
    .events = (struct wl_message[]){{"ping", "u", nullptr}},
};

/**
 * @copydoc xdg_wm_base_listener::ping
 */
static void ping(void *, void *b, uint32_t s)
{
    // xdg_wm_base_pong
    (void)wl_proxy_marshal_flags((struct wl_proxy *)b, 3, nullptr,
                                 wl_proxy_get_version((struct wl_proxy *)b), 0,
                                 s);
}

/**
 * @brief An interface for handling events sent from the XDG "registry" object,
 * @c wm_base. This is basically a nerdy game of table tennis, we just recieve a
 * ping and send back a pong, over and over again so the WM doesn't think we're
 * a zombie.
 * @since v0.0.1
 */
struct xdg_wm_base_listener
{
    /**
     * @brief The ping event asks the client if it’s still alive. Pass the
     * serial specified in the event back to the compositor by sending a “pong”
     * request back with the specified serial. A compositor is free to ping in
     * any way it wants, but a client must always respond to any xdg_wm_base
     * object it created.
     * @since v0.0.1
     *
     * @remark It’s unspecified what will happen if the client doesn’t respond
     * to the ping request, or in what timeframe.
     *
     * @param[in] data Any data to be sent alongside events.
     * @param[in] base The window mananger base object that generated the event.
     * @param[in] serial The serial code of the event. The client must respond
     * to the ping with this serial code in its pong.
     */
    void (*ping)(void *data, void *base, uint32_t serial);
};

/**
 * @copydoc xdg_surface_listener::configure
 */
static void configure(void *d, void *t, uint32_t s)
{
    // Acknowlege the configuration. (xdg_surface_ack_configure)
    (void)wl_proxy_marshal_flags((struct wl_proxy *)t, 4, nullptr,
                                 wl_proxy_get_version((struct wl_proxy *)t), 0,
                                 s);
    wl_surface_commit(d);
    waterlily_engine_log(SUCCESS, "Configure request completed.");
}

/**
 * @brief An interface for handling events for the @c xdg_surface wrapper around
 * the @c wl_surface object.
 * @since v0.0.1
 */
struct xdg_surface_listener
{
    /**
     * @brief The configure event marks the end of a configure sequence. A
     * configure sequence is a set of one or more events configuring the state
     * of the xdg_surface. Clients should send an ack_configure before
     * committing the new surface.
     * @since v0.0.1
     *
     * @remark Where applicable, xdg_surface surface roles will during a
     * configure sequence extend this event as a latched state sent as events
     * before the xdg_surface.configure event. If the client receives multiple
     * configure events before it can respond to one, it is free to discard all
     * but the last event it received.
     *
     * @param[in] data Any user-defined data sent alongside the surface.
     * @param[in] surface The surface who is being configured.
     * @param[in] serial The serial ID of this event. This should be sent
     * alongside a configuration acknowledgement event.
     */
    void (*configure)(void *data, void *surface, uint32_t serial);
};

/**
 * @copydoc xdg_toplevel_listener::topConfigure
 */
static void topConfigure(void *d, void *, int32_t w, int32_t h,
                         struct wl_array *s)
{
    waterlily_engine_log(INFO, "Configure request recieved.");

    waterlily_context_t *context = d;
    uint32_t width = w * context->window.scale;
    uint32_t height = h * context->window.scale;
    if (context->window.extent.width != width &&
        context->window.extent.height != height)
    {
        context->window.resized = true;
        context->window.extent.width = width;
        context->window.extent.height = height;
        waterlily_vulkan_getExtentSurface(context);
        waterlily_engine_log(INFO, "Window dimensions adjusted: %dx%d.",
                             context->window.extent.width,
                             context->window.extent.height);
    }

    int32_t *i;
    wl_array_for_each(i, s)
    {
        switch (*i)
        {
            case 2:
                waterlily_engine_log(INFO, "The window is now fullscreened.");
                break;
            case 4:
                waterlily_engine_log(INFO, "The window is now activated.");
                break;
            case 9:
                waterlily_engine_log(INFO, "The window is now suspended.");
                break;
            default:
                waterlily_engine_log(ERROR, "Got unknown state value '%d'.",
                                     *i);
                break;
        }
    }
}

/**
 * @copydoc xdg_toplevel_listener::close
 */
static void closeToplevel(void *d, void *)
{
    waterlily_context_t *context = d;
    context->window.close = true;
}

/**
 * @copydoc xdg_toplevel_listener::bounds
 */
static void bounds(void *, void *, int32_t, int32_t) {}

/**
 * @copydoc xdg_toplevel_listener::capabilities
 */
static void capabilities(void *, void *, struct wl_array *c)
{
    int32_t *i;
    wl_array_for_each(i, c) if (*i == 3)
    {
        waterlily_engine_log(SUCCESS, "Found fullscreen support.");
        return;
    }

    waterlily_engine_log(ERROR, "No fullscreen support available.");
}

/**
 * @brief An interface for handling events for the @c xdg_toplevel structure.
 * This provides a bunch of information about surface dimensions, capabilities,
 * and more.
 * @since v0.0.1
 */
struct xdg_toplevel_listener
{
    /**
     * @brief This configure event asks the client to resize its toplevel
     * surface or to change its state. The width and height arguments specify a
     * hint to the window about how its surface should be resized in window
     * geometry coordinates. The states listed in the event specify how the
     * width/height arguments should be interpreted, and possibly how it should
     * be drawn.
     * @since v0.0.1
     *
     * @remark If the width or height arguments are zero, it means the client
     * should decide its own window dimension. This may happen when the
     * compositor needs to configure the state of the surface but doesn’t have
     * any information about any previous or expected dimension.
     *
     * @param[in] data Any data asked to have been sent alongside the toplevel.
     * @param[in] toplevel The toplevel this configuration event was sent for.
     * @param[in] width The suggested width of the window in screen coordinates.
     * @param[in] height The suggested height of the window in screen
     * coordinates.
     * @param[in] states An array of states to use for dealing with the provided
     * width and height or general window rendering.
     */
    void (*topConfigure)(void *data, void *toplevel, int32_t width,
                         int32_t height, struct wl_array *states);

    /**
     * @brief The close event is sent by the compositor when the user wants the
     * surface to be closed.
     * @since v0.0.1
     *
     * @remark This is only a request that the user intends to close the window.
     * The client may choose to ignore this request, or show a dialog to ask the
     * user to save their data, etc.
     *
     * @param[in] data Any data asked to have been sent alongside the toplevel.
     * @param[in] toplevel The toplevel that wishes to be closed.
     */
    void (*close)(void *data, void *toplevel);

    /**
     * @brief The configure_bounds event may be sent prior to a configure event
     * to communicate the bounds a window geometry size is recommended to
     * constrain to. The bounds can for example correspond to the size of a
     * monitor excluding any panels or other shell components, so that a surface
     * isn't created in a way that it cannot fit.
     * @since v0.0.1
     *
     * @remarks If width and height are 0, it means bounds is unknown and
     * equivalent to as if no configure_bounds event was ever sent for this
     * surface.
     *
     * @param[in] data Any data asked to have been sent alongside the toplevel.
     * @param[in] toplevel The toplevel this event was sent for.
     * @param[in] width The constrain width in screen coordinates.
     * @param[in] height The constrain height in screen coordinates.
     */
    void (*bounds)(void *data, void *toplevel, int32_t width, int32_t height);

    /**
     * @brief This event advertises the capabilities supported by the
     * compositor. If a capability isn't supported, clients should hide or
     * disable the UI elements that expose this functionality. The
     * compositor will ignore requests it doesn't support. Compositors must send
     * this event once before the first xdg_surface.configure event. When the
     * capabilities change, compositors must send this event again and then send
     * a configure event.
     * @since v0.0.1
     *
     * @remarks The capabilities are sent as an array of 32-bit unsigned
     * integers in native endianness.
     *
     * @param[in] data Any data asked to have been sent alongside the toplevel.
     * @param[in] toplevel The toplevel this event was sent for.
     * @param[in] capabilities An array of the compositor's advertised
     * capabilities.
     */
    void (*capabilities)(void *data, void *toplevel,
                         struct wl_array *capabilities);
};

static void geometry(void *, struct wl_output *, int32_t, int32_t, int32_t,
                     int32_t, int32_t, const char *, const char *, int32_t)
{
}

static void mode(void *, struct wl_output *, uint32_t, int32_t, int32_t,
                 int32_t)
{
}

static void finish(void *, struct wl_output *) {}

static void scale(void *d, struct wl_output *, int32_t s)
{
    waterlily_context_t *context = d;
    context->window.scale = s;
    waterlily_engine_log(INFO, "Monitor scale %d.", context->window.scale);
}

static void name(void *, struct wl_output *, const char *) {}

static void description(void *, struct wl_output *, const char *) {}

static void keymap(void *d, struct wl_keyboard *, uint32_t f, int32_t m,
                   uint32_t s)
{
    waterlily_context_t *context = d;

    if (f != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        waterlily_engine_log(ERROR, "Got unsupported keyboard format '%d'.", f);
        context->window.close = true;
        return;
    }

    char *map_shm = mmap(NULL, s, PROT_READ, MAP_PRIVATE, m, 0);
    if (map_shm == MAP_FAILED)
    {
        waterlily_engine_log(ERROR, "Failed to map keymap file.");
        context->window.close = true;
        return;
    }

    waterlily_input_setKeymap(context, map_shm);
    munmap(map_shm, s);
    close(m);
}

static void enter(void *, struct wl_keyboard *, uint32_t, struct wl_surface *,
                  struct wl_array *)
{
}

static void leave(void *, struct wl_keyboard *, uint32_t, struct wl_surface *)
{
}

void key(void *d, struct wl_keyboard *, uint32_t, uint32_t t, uint32_t k,
         uint32_t s)
{
    waterlily_context_t *context = d;
    if (t - context->input.down[0].timestamp > WATERLILY_KEY_TIMER_MS)
    {
        context->input.down[0] = (waterlily_key_t){
            t,
            xkb_state_key_get_one_sym(context->input.state, k + 8),
            s,
        };
        return;
    }
    else if (t - context->input.down[1].timestamp > WATERLILY_KEY_TIMER_MS)
    {
        context->input.down[1] = (waterlily_key_t){
            t,
            xkb_state_key_get_one_sym(context->input.state, k + 8),
            s,
        };
        return;
    }
}

void modifiers(void *d, struct wl_keyboard *, uint32_t, uint32_t p, uint32_t a,
               uint32_t l, uint32_t g)
{
    waterlily_context_t *context = d;
    waterlily_input_updateModifiers(context, p, a, l, g);
}

void repeatInfo(void *, struct wl_keyboard *, int32_t, int32_t) {}

static void capabilitiesChange(void *, struct wl_seat *, uint32_t) {}

static void seatName(void *d, struct wl_seat *, const char *name)
{
    waterlily_engine_log(INFO, "Seat named '%s'.", name);

    waterlily_context_t *context = d;
    context->window.data.keyboard =
        wl_seat_get_keyboard(context->window.data.seat);
    struct wl_keyboard_listener keyboardListener = {
        keymap, enter, leave, key, modifiers, repeatInfo};
    wl_keyboard_add_listener(context->window.data.keyboard, &keyboardListener,
                             context);
}

static void global(void *d, struct wl_registry *registry,
                   uint32_t interfaceName, const char *interface,
                   uint32_t version)
{
    waterlily_context_t *context = d;

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        context->window.data.compositor = wl_registry_bind(
            registry, interfaceName, &wl_compositor_interface, version);
        waterlily_engine_log(SUCCESS, "Connected to compositor v%d.", version);
        return;
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        context->window.data.shell = wl_registry_bind(
            registry, interfaceName, &pXDGShellInterface, version);
        struct xdg_wm_base_listener shellListener = {&ping};
        // xdg_wm_base_add_listener
        (void)wl_proxy_add_listener(
            (struct wl_proxy *)context->window.data.shell,
            (void (**)(void))&shellListener, nullptr);
        waterlily_engine_log(SUCCESS, "Connected to window manager v%d.",
                             version);
        return;
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        context->window.data.output = wl_registry_bind(
            registry, interfaceName, &wl_output_interface, version);
        struct wl_output_listener outputListener = {
            &geometry, &mode, &finish, &scale, &name, &description};
        (void)wl_output_add_listener(context->window.data.output,
                                     &outputListener, context);
        waterlily_engine_log(SUCCESS, "Connected to output device v%d.",
                             version);
        return;
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        context->window.data.seat = wl_registry_bind(
            registry, interfaceName, &wl_seat_interface, version);
        struct wl_seat_listener seatListener = {&capabilitiesChange, &seatName};
        (void)wl_seat_add_listener(context->window.data.seat, &seatListener,
                                   context);
        waterlily_engine_log(SUCCESS, "Connected to seat device v%d.", version);
        return;
    }

    waterlily_engine_log(INFO, "Found unknown interface '%s'.", interface);
}

static void globalRemove(void *, struct wl_registry *, uint32_t) {}

bool waterlily_window_create(const char *const title,
                             waterlily_context_t *context)
{
    context->window.data.display = wl_display_connect(nullptr);
    if (__builtin_expect(context->window.data.display == nullptr, false))
    {
        waterlily_engine_log(ERROR, "Failed to connect to display server.");
        return false;
    }

    context->window.data.registry =
        wl_display_get_registry(context->window.data.display);
    struct wl_registry_listener registryListener = {&global, &globalRemove};
    (void)wl_registry_add_listener(context->window.data.registry,
                                   &registryListener, context);
    (void)wl_display_roundtrip(context->window.data.display);

    context->window.data.surface =
        wl_compositor_create_surface(context->window.data.compositor);
    // xdg_wm_base_get_xdg_surface
    context->window.data.shellSurface =
        (struct xdg_surface *)wl_proxy_marshal_flags(
            (struct wl_proxy *)context->window.data.shell, 2,
            &pXDGSurfaceInterface,
            wl_proxy_get_version((struct wl_proxy *)context->window.data.shell),
            0, nullptr, context->window.data.surface);
    struct xdg_surface_listener shellSurfaceListener = {&configure};
    // xdg_surface_add_listener
    (void)wl_proxy_add_listener(
        (struct wl_proxy *)context->window.data.shellSurface,
        (void (**)(void))&shellSurfaceListener, context->window.data.surface);
    // xdg_surface_get_toplevel
    context->window.data.toplevel =
        (struct xdg_toplevel *)wl_proxy_marshal_flags(
            (struct wl_proxy *)context->window.data.shellSurface, 1,
            &pXDGToplevelInterface,
            wl_proxy_get_version(
                (struct wl_proxy *)context->window.data.shellSurface),
            0, nullptr);
    struct xdg_toplevel_listener toplevelListener = {
        &topConfigure, &closeToplevel, &bounds, &capabilities};
    // xdg_toplevel_add_listener
    (void)wl_proxy_add_listener(
        (struct wl_proxy *)context->window.data.toplevel,
        (void (**)(void))&toplevelListener, context);

    // xdg_toplevel_set_title
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.toplevel, 2, nullptr,
        wl_proxy_get_version((struct wl_proxy *)context->window.data.toplevel),
        0, title);
    // xdg_toplevel_set_app_id
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.toplevel, 3, nullptr,
        wl_proxy_get_version((struct wl_proxy *)context->window.data.toplevel),
        0, title);
    // xdg_toplevel_set_fullscreen
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.toplevel, 11, nullptr,
        wl_proxy_get_version((struct wl_proxy *)context->window.data.toplevel),
        0, context->window.data.output);
    waterlily_engine_log(INFO, "Setup window properly.");

    return true;
}

void waterlily_window_destroy(waterlily_context_t *context)
{
    // xdg_toplevel_destroy
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.toplevel, 0, nullptr,
        wl_proxy_get_version((struct wl_proxy *)context->window.data.toplevel),
        WL_MARSHAL_FLAG_DESTROY);
    // xdg_surface_destroy
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.shellSurface, 0, nullptr,
        wl_proxy_get_version(
            (struct wl_proxy *)context->window.data.shellSurface),
        WL_MARSHAL_FLAG_DESTROY);
    // xdg_wm_base_destroy
    (void)wl_proxy_marshal_flags(
        (struct wl_proxy *)context->window.data.shell, 0, nullptr,
        wl_proxy_get_version((struct wl_proxy *)context->window.data.shell),
        WL_MARSHAL_FLAG_DESTROY);

    wl_surface_destroy(context->window.data.surface);
    wl_compositor_destroy(context->window.data.compositor);
    wl_output_release(context->window.data.output);
    wl_keyboard_release(context->window.data.keyboard);
    wl_seat_release(context->window.data.seat);
    wl_registry_destroy(context->window.data.registry);
    wl_display_disconnect(context->window.data.display);
}

bool waterlily_window_process(waterlily_context_t *context)
{
    return wl_display_dispatch_pending(context->window.data.display) != -1 &&
           !context->window.close;
}

