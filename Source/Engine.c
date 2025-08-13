#include "Internal.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>

void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...)
{
    static const char *const tags[] = {
        [WATERLILY_LOG_TYPE_INFO] = "INFO",
        [WATERLILY_LOG_TYPE_SUCCESS] = " OK ",
        [WATERLILY_LOG_TYPE_WARNING] = "WARN",
        [WATERLILY_LOG_TYPE_ERROR] = "FAIL",
    };

    FILE *output = data->type < WATERLILY_LOG_TYPE_WARNING ? stdout : stderr;
    (void)fprintf(output, "[%s] %-15s ln. %04zu: ", tags[data->type],
                  data->filename, data->line);

    va_list args;
    va_start(args);
    (void)vfprintf(output, format, args);
    (void)fputc('\n', output);
    va_end(args);

    if (data->type == WATERLILY_LOG_TYPE_ERROR)
        (void)fprintf(output, "\tCurrent ERRNO (may be garbage): %d\n", errno);
}

bool waterlily_engine_digest(waterlily_context_t *context, int argc,
                             const char *const *const argv,
                             char **workingDirectory,
                             size_t *workingDirectoryLength)
{
    *workingDirectory = (char *)argv[0];
    *workingDirectoryLength = strrchr(*workingDirectory, '/') - argv[0];

    for (size_t i = 1; i < (size_t)argc; ++i)
    {
        char *currentArg = (char *)argv[i];

        if (*currentArg != '-' || *(currentArg + 1) != '-')
        {
            waterlily_engine_log(
                ERROR, "Argument '%s' malformed (no '--' prefix).", currentArg);
            return false;
        }
        currentArg += 2;

        if (strcmp(currentArg, "help") == 0)
        {
            waterlily_engine_log(
                INFO, "Usage: app [OPTIONS]\nOptions:\n\t--help: Display this "
                      "help message and exit.\n\t--version: Display engine "
                      "version information and exit.\n\t--license: Display "
                      "licensing information and exit.\n\n\t--fps: Display an "
                      "FPS counter once in-game.");
            return false;
        }
        else if (strcmp(currentArg, "version") == 0)
        {
            waterlily_engine_log(INFO,
                                 "Waterlily version " WATERLILY_VERSION ".");
            return false;
        }
        else if (strcmp(currentArg, "license") == 0)
        {
            waterlily_engine_log(
                INFO, "Copyright (c) 2025 - Israfil Argos\nThis software is "
                      "under the GPLv3. This program may be repackaged and "
                      "redistributed under the terms of that license.\nYou "
                      "should have recieved a copy of the license alongside "
                      "your copy of this program,\nbut if you did not, you can "
                      "find it at <https://www.gnu.org/licenses/gpl-3.0.txt>");
            return false;
        }
        else if (strcmp(currentArg, "fps") == 0)
            context->arguments.displayFPS = true;
    }

    waterlily_engine_log(SUCCESS, "Parsed all provided arguments.");
    return true;
}

bool waterlily_engine_configure(waterlily_configuration_t *configuration)
{
    FILE *file;
    if (!waterlily_files_open("Assets/engine.waterlily", &file))
        return false;

    size_t fileSize;
    if (!waterlily_files_measure(file, &fileSize))
        return false;

    char contents[fileSize + 1];
    if (!waterlily_files_read(file, fileSize, (uint8_t *)contents))
        return false;
    contents[fileSize] = 0;
    waterlily_files_close(file);

    char *cursor = (char *)contents;
    char *segment = cursor;
    uint8_t search = 0;
    while (*cursor != 0)
    {
        if (*cursor != ':' && *cursor != '\n')
        {
            cursor++;
            continue;
        }

        *cursor = 0;
        cursor++;
        switch (search)
        {
            case 0:
                break;
            case 1:
                configuration->title = segment;
                break;
        }

        if (strcmp(segment, "title") == 0)
        {
            search = 1;
            continue;
        }

        waterlily_engine_log(ERROR, "Found unknown configuration key '%s'.",
                             segment);
        return false;
    }

    return true;
}

