#include <WaterlilyRaw.h>
#include <xkbcommon/xkbcommon.h>

bool waterlily_input_createContext(waterlily_context_t *context)
{
    context->input.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if(context->input.context == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to create input context.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created input context.");
    return true;
}
