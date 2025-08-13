#include "Internal.h"
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

bool waterlily_files_execute(char *const *args)
{
    int pid = fork();
    if (pid < 0)
    {
        waterlily_engine_log(ERROR, "Failed to fork process, code %d.", pid);
        return false;
    }

    if (pid == 0)
    {
        if (args[0] == nullptr)
            return false;
        (void)execve(args[0], args, nullptr);
        waterlily_engine_log(
            ERROR, "Failed to replace current process, code %d.", errno);
        return false;
    }

    int status;
    int waitStatus = waitpid(pid, &status, 0);
    if (waitStatus < 0)
    {
        waterlily_engine_log(ERROR, "Child process was killed via signal.");
        return false;
    }

    if (status == 1)
    {
        waterlily_engine_log(ERROR, "Child process exited abnormally.");
        return false;
    }

    return true;
}

bool waterlily_files_exists(const char *const path)
{
    FILE *file;
    if (!waterlily_files_open(path, &file))
        return false;
    waterlily_files_close(file);
    return true;
}

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

bool waterlily_files_open(const char *const path, FILE **file)
{
    *file = fopen(path, "rb");
    if (*file == nullptr)
    {
        waterlily_engine_log(WARNING, "Failed to open file '%s'.", path);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Opened file '%s'.", path);
    return true;
}

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

