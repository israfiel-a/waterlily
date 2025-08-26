#include <internal/config.h>
#include <internal/files.h>
#include <internal/logging.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct waterlily_configuration config = {0};

static void digestArguments(int argc, const char *const *const argv)
{
    char *workingDirectory = (char *)argv[0];
    size_t workingDirectoryLength =
        strrchr(workingDirectory, '/') - workingDirectory;
    char wd[workingDirectoryLength + 1];
    (void)strncpy(wd, workingDirectory, workingDirectoryLength);
    wd[workingDirectoryLength] = 0;

    int status = chdir(wd);
    if (status < 0)
        waterlily_report("Failed to move working directory into '%s'.", wd);
    waterlily_log(SUCCESS, "Changed working directory to '%s'.", wd);

    for (size_t i = 1; i < (size_t)argc; ++i)
    {
        char *currentArg = (char *)argv[i];

        if (*currentArg != '-' || *(currentArg + 1) != '-')
            waterlily_report("Argument '%s' malformed (no '--' prefix).",
                             currentArg);
        currentArg += 2;

        if (strcmp(currentArg, "help") == 0)
        {
            waterlily_log(
                INFO, "Usage: app [OPTIONS]\nOptions:\n\t--help: Display this "
                      "help message and exit.\n\t--license: Display licensing "
                      "information and exit.\n\n\t--fps: Display an FPS "
                      "counter once in-game.");
            exit(0);
        }
        else if (strcmp(currentArg, "license") == 0)
        {
            waterlily_log(
                INFO, "Copyright (c) 2025 - Israfil Argos\nThis software is "
                      "under the GPLv3. This program may be repackaged and "
                      "redistributed under the terms of that license.\nYou "
                      "should have recieved a copy of the license alongside "
                      "your copy of this program,\nbut if you did not, you can "
                      "find it at <https://www.gnu.org/licenses/gpl-3.0.txt>");
            exit(0);
        }
        else if (strcmp(currentArg, "fps") == 0)
            config.arguments.displayFPS = true;
    }

    waterlily_log(SUCCESS, "Parsed all provided arguments.");
}

static void readConfiguration(void)
{
    waterlily_file_t file = {
        .name = "engine",
        .type = WATERLILY_CONFIG_FILE,
    };
    waterlily_readFile(&file);

    for (size_t i = 0; i < file.config.pairCount; ++i)
    {
        auto readConfig = file.config.pairs[i];
        switch (readConfig.key)
        {
            case WATERLILY_CONFIG_TITLE_KEY:
                config.title = readConfig.value.title;
                break;
            case WATERLILY_CONFIG_AUTHOR_KEY:
                config.author = readConfig.value.author;
                break;
            case WATERLILY_CONFIG_VERSION_KEY:
                config.version = readConfig.value.author;
                break;
            default:
                waterlily_report("Got unknown engine configuration key '%d'.",
                                 readConfig.key);
        }
    }
}

struct waterlily_configuration *
waterlily_initializeConfiguration(int argc, const char *const *const argv)
{
    digestArguments(argc, argv);
    readConfiguration();
    return &config;
}

