#include <WaterlilyRaw.h>

bool waterlily_files_exists(const char *const path)
{
    FILE *file;
    if (!waterlily_files_open(path, &file))
        return false;
    waterlily_files_close(file);
    return true;
}

