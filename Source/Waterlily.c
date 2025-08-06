#include <Waterlily.h>

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
    if(!waterlily_changeDirectory(path)) return false;

    char* title = nullptr;
    uint32_t version = 0;
    for(size_t i = 1; i < (size_t)argc; i++) {
        const char* const arg = argv[i];
        
        if(title == nullptr) title = (char*)arg;
        else version = stringToNumber(arg); 
    }

    if (!waterlily_createWindow(title)) return false;

    const char *defaults[2] = {"default.vert", "default.frag"};
    if (!waterlily_compileShaders(defaults, 2)) return false;

    if (!waterlily_create(title, version)) return false;

    waterlily_log(SUCCESS, "Initialized engine.");
    return true;
}

void waterlily_run(void)
{
    uint32_t width, height;
    waterlily_getSizeWindow(&width, &height);
    while (waterlily_processWindow())
    {
        if (!waterlily_render(width, height)) return;
        if (!waterlily_sync()) return;
    }
}

void waterlily_cleanup(void)
{
    waterlily_destroy();
    waterlily_destroy();
    waterlily_log(SUCCESS, "Cleaned up engine.");
}
