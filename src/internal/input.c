#include <internal/input.h>
#include <internal/logging.h>

static struct waterlily_input_context context = {0};

static void createKeymap(const char *const string)
{
    struct xkb_keymap *map = xkb_keymap_new_from_string(
        context.handle, string, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (map == nullptr)
        waterlily_report("Failed to create new keymap.");
    waterlily_log(SUCCESS, "Created a new keymap.");

    context.state = xkb_state_new(map);
    if (context.state == nullptr)
        waterlily_report("Failed to create new XKB state object.");
    waterlily_log(SUCCESS, "Created a new XKB state object.");
}

void waterlily_createInputContext(const char *const string)
{
    context.handle = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (context.handle == nullptr)
        waterlily_report("Failed to create input context.");
    waterlily_log(SUCCESS, "Created input context.");

    createKeymap(string);
}

void waterlily_destroyInputContext(void)
{
    // There's a pretty good chance the application dies before getting to this
    // point, so we need to make sure it doesn't segfault.
    if (context.state != nullptr)
    {
        struct xkb_keymap *keymap = xkb_state_get_keymap(context.state);
        xkb_state_unref(context.state);
        xkb_keymap_unref(keymap);
    }
    xkb_context_unref(context.handle);
}

