#include <Waterlily.h>
#define PRIMROSE_IMPLEMENTATION
#include <Primrose.h>
#include <Ageratum.h>
#include <Geranium.h>
#include <Hyacinth.h>

// get rid of this
static uint32_t stringToNumber(const char *const string) {
    uint32_t value = 0;
    
    char* stringCopy = (char*)string;
    while(*stringCopy++ != 0)
        switch(*stringCopy) {
            case '0': value *= 10; value += 0; break;
            case '1': value *= 10; value += 1; break;
            case '2': value *= 10; value += 2; break;
            case '3': value *= 10; value += 3; break;
            case '4': value *= 10; value += 4; break;
            case '5': value *= 10; value += 5; break;
            case '6': value *= 10; value += 6; break;
            case '7': value *= 10; value += 7; break;
            case '8': value *= 10; value += 8; break;
            case '9': value *= 10; value += 9; break;
            default:  break;
        }

    return value;
}

bool waterlily_initialize(int argc, const char *const *const argv)
{
    const char* const path = argv[0];
    if(!ageratum_changeDirectory(path)) return false;

    char* title = nullptr;
    uint32_t version = 0;
    for(size_t i = 1; i < (size_t)argc; i++) {
        const char* const arg = argv[i];
        
        if(title == nullptr) title = (char*)arg;
        else version = stringToNumber(arg); 
    }

    if (!hyacinth_create(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!geranium_compileShaders(defaults, 2)) return false;

    if (!geranium_create(title, version)) return false;

    primrose_log(SUCCESS, "Initialized engine.");
    return true;
}

void waterlily_run(void)
{
    uint32_t width, height;
    hyacinth_getSize(&width, &height);
    while (hyacinth_process())
    {
        if (!geranium_render(width, height)) return;
        if (!geranium_sync()) return;
    }
}

void waterlily_cleanup(void)
{
    geranium_destroy();
    hyacinth_destroy();
    primrose_log(SUCCESS, "Cleaned up engine.");
}
