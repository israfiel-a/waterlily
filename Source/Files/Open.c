#include <Waterlily.h>

bool waterlily_files_open(const char *const path, FILE **file)
{
    *file = fopen(path, "rb");
    if (*file == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open file '%s'.", path);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Opened file '%s'.", path);
    return true;
}

