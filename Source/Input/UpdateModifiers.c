#include <Waterlily.h>

void waterlily_input_updateModifiers(waterlily_context_t *context,
                                     uint32_t depressed, uint32_t latched,
                                     uint32_t locked, uint32_t group)
{
    xkb_state_update_mask(context->input.state, depressed, latched, locked, 0,
                          0, group);
    waterlily_engine_log(INFO, "Updated keyboard modifier state.");
}

