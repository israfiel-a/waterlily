#include "internal.h"

void waterlily_input_checkKeys(waterlily_context_t *context)
{
    waterlily_key_t *first = &context->input.down[0];
    waterlily_key_t *second = &context->input.down[1];

    bool firstValid = first->timestamp > WATERLILY_KEY_TIMER_MS;
    bool secondValid = second->timestamp > WATERLILY_KEY_TIMER_MS;

    // There ain't no key to combo.
    if (!firstValid && !secondValid)
        return;

    for (size_t i = 0; i < context->input.combinationCount; ++i)
    {
        waterlily_key_combination_t *combo = &context->input.combinations[i];
        bool secondGiven = combo->second.symbol != XKB_KEY_NoSymbol;
        // If there's a second symbol given but no second symbol pressed,
        // ignore.
        if (secondGiven && !secondValid)
            continue;
        // There aren't enough valid pressed keys.
        if (secondGiven && !firstValid)
            continue;

        if (firstValid && combo->first.symbol == first->symbol &&
            combo->first.state == first->state)
        {
            if (!secondGiven)
            {
                combo->func(first, second, context);
                return;
            }

            if (combo->second.symbol == second->symbol &&
                combo->second.state == second->state)
            {
                combo->func(first, second, context);
                return;
            }
        }
        else if (!firstValid && combo->first.symbol == second->symbol &&
                 combo->first.state == second->state)
            combo->func(second, first, context);
    }
}

bool waterlily_input_createContext(waterlily_context_t *context)
{
    context->input.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (context->input.context == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to create input context.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created input context.");
    return true;
}

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

void waterlily_input_updateModifiers(waterlily_context_t *context,
                                     uint32_t depressed, uint32_t latched,
                                     uint32_t locked, uint32_t group)
{
    xkb_state_update_mask(context->input.state, depressed, latched, locked, 0,
                          0, group);
    waterlily_engine_log(INFO, "Updated keyboard modifier state.");
}

