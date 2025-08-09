#include <WaterlilyRaw.h>
#include <sys/stat.h>

bool waterlily_files_measure(FILE *file, size_t *length)
{
    struct stat stats = {0};
    int descriptor = fileno(file);
    if (fstat(descriptor, &stats) == -1)
    {
        waterlily_engine_log(ERROR, "Failed to stat file at descriptor %d.",
                             descriptor);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Measured file at descriptor %d, %zu bytes.",
                         descriptor, stats.st_size);
    *length = stats.st_size;
    return true;
}

