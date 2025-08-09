#include <WaterlilyRaw.h>

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

