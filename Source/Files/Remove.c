#include <WaterlilyRaw.h>
#include <dirent.h>
#include <string.h>

bool waterlily_files_remove(const char *const path)
{
    DIR *const directory = opendir(path);
    if (directory)
    {
        struct dirent *entry;
        while ((entry = readdir(directory)))
        {
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;

            char filename[strlen(path) + strlen(entry->d_name) + 2];
            sprintf(filename, "%s/%s", path, entry->d_name);
            if ((entry->d_type == DT_DIR &&
                 !waterlily_files_remove(filename)) ||
                remove(filename) != 0)
            {
                closedir(directory);
                return false;
            }
        }
        if (closedir(directory) == -1)
            return false;
    }
    return remove(path) == 0;
}

