#include <Waterlily.h>

bool waterlily_files_read(FILE *file, size_t count, uint8_t *buffer)
{
    size_t read = fread(buffer, 1, count, file);
    if (read < count)
    {
        waterlily_engine_log(ERROR,
                             "Failed to read the requested %zu bytes from file "
                             "at descriptor %d, read %zu instead.",
                             count, fileno(file), read);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Read %zu bytes from file at descriptor %d.",
                         count, fileno(file));
    return true;
}

