#include <WaterlilyRaw.h>

bool waterlily_input_setKeymap(waterlily_context_t *context,
                               const char *const string)
{
    struct xkb_keymap *map = xkb_keymap_new_from_string(
        context->input.context, string, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (map == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to create new keymap.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created a new keymap.");

    context->input.state = xkb_state_new(map);
    if (context->input.state == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to create new XKB state object.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created a new XKB state object.");

    return true;
}

