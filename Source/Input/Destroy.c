#include <WaterlilyRaw.h>

void waterlily_input_destroy(waterlily_context_t *context)
{
    // There's a pretty good chance the application dies before getting to this
    // point, so we need to make sure it doesn't segfault.
    if (context->input.state != nullptr)
    {
        struct xkb_keymap *keymap = xkb_state_get_keymap(context->input.state);
        xkb_state_unref(context->input.state);
        xkb_keymap_unref(keymap);
    }
    xkb_context_unref(context->input.context);
}

