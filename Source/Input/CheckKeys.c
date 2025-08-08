#include <Waterlily.h>

void waterlily_input_checkKeys(waterlily_context_t *context,
                               waterlily_key_combination_t *keys, size_t count)
{
    waterlily_key_t *first = &context->input.down[0];
    waterlily_key_t *second = &context->input.down[1];

    bool firstValid = first->timestamp > WATERLILY_KEY_TIMER_MS;
    bool secondValid = second->timestamp > WATERLILY_KEY_TIMER_MS;

    // There ain't no key to combo.
    if (!firstValid && !secondValid)
        return;

    for (size_t i = 0; i < count; ++i)
    {
        waterlily_key_combination_t *combo = &keys[i];
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

