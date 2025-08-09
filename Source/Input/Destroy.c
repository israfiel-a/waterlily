#include <WaterlilyRaw.h>

void waterlily_input_destroy(waterlily_context_t *context)
{
    struct xkb_keymap *keymap = xkb_state_get_keymap(context->input.state);
    xkb_state_unref(context->input.state);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context->input.context);
}

