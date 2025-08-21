#include "internal/files.h"

#include "internal.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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
    if (!waterlily_openFile(path, WATERLILY_GENERIC_FILE, &file, nullptr))
        return false;
    waterlily_closeFile(file);
    return true;
}

bool waterlily_openFile(const char *const name, waterlily_file_type_t type,
                        FILE **file, size_t *size)
{
    static const char *const typeSubdirectories[] = {
        [WATERLILY_GENERIC_FILE] = ".",
        [WATERLILY_CONFIG_FILE] = ".",
    };
    static const char *const typeExtensions[] = {
        [WATERLILY_GENERIC_FILE] = "",
        [WATERLILY_CONFIG_FILE] = ".wl",
    };
    const char *const subdirectory = typeSubdirectories[type];
    const char *const extension = typeExtensions[type];

    size_t filepathLength = sizeof(WATERLILY_RESOURCE_DIRECTORY) +
                            strlen(name) + strlen(subdirectory);
    char filepath[filepathLength];
    (void)snprintf(filepath, filepathLength,
                   WATERLILY_RESOURCE_DIRECTORY "%s/%s%s", subdirectory, name,
                   extension);

    *file = fopen(filepath, "rb");
    if (*file == nullptr)
    {
        waterlily_engine_log(WARNING, "Failed to open file '%s'.", filepath);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Opened file '%s'.", filepath);

    if (size != nullptr)
    {
        struct stat stat;
        if (fstat(fileno(*file), &stat) == -1)
        {
            waterlily_engine_log(ERROR, "Failed to stat file '%s'.", filepath);
            return false;
        }
        *size = stat.st_size;
        waterlily_engine_log(SUCCESS, "Statted file '%s'.", filepath);
    }

    return true;
}

static bool parseConfig(const char *const raw,
                        waterlily_file_contents_t *contents)
{
    char *key = (char *)raw;
    char *value = nullptr;

    typeof(contents->config.pairs[0].key) keyType = 0;

    for (char *ch = key; ch != 0; ++ch)
    {
        if (*ch != '\n' && *ch == '=')
        {
            if (value != nullptr)
            {
                *ch = 0;
                waterlily_engine_log(ERROR, "Got double key ('%s').", key);
                return false;
            }

            if (strncmp(key, "title", 5) == 0)
                keyType = WATERLILY_CONFIG_FILE_TITLE_KEY;
            else if (strncmp(key, "shaders", 7) == 0)
                keyType = WATERLILY_CONFIG_FILE_SHADERS_KEY;
            else
            {
                *ch = 0;
                waterlily_engine_log(
                    ERROR, "Found unknown configuration key '%s'.", key);
                return false;
            }
            value = ch;
            continue;
        }
        else if (*ch != '\n')
            continue;

        if (value == nullptr)
        {
            *ch = 0;
            waterlily_engine_log(ERROR, "No value given to key '%s'.", key);
            return false;
        }

        switch (keyType)
        {
            case WATERLILY_CONFIG_FILE_TITLE_KEY:
                contents->config.pairs[contents->config.pairCount] =
                    (typeof(contents->config.pairs[0])){
                        .key = keyType,
                        .value.title = strndup(value, ch - value),
                    };
                contents->config.pairCount++;
                break;
            default:
                waterlily_engine_log(ERROR, "Unimplement configuration key %d.",
                                     keyType);
                return false;
        }
    }

    return true;
}

bool waterlily_interpretFile(FILE *file, size_t size,
                             waterlily_file_type_t type, char *raw,
                             waterlily_file_contents_t *contents)
{
    size_t read = fread(raw, 1, size, file);
    if (read < size)
    {
        waterlily_engine_log(ERROR,
                             "Failed to read the requested %zu bytes from file "
                             "at descriptor %d, read %zu instead.",
                             size, fileno(file), read);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Read %zu bytes from file at descriptor %d.",
                         size, fileno(file));

    switch (type)
    {
        case WATERLILY_CONFIG_FILE:
            raw[size] = 0;
            return parseConfig(raw, contents);
        default:
            return true;
    }
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

