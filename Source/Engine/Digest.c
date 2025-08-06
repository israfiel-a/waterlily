#include <Waterlily.h>
#include <string.h>

bool waterlily_engine_digest(int argc, const char *const *const argv,
                             waterlily_arguments_t *arguments)
{
    const char *const rawExecutable = argv[0];
    arguments->requiredDirectory = (char *)rawExecutable;
    arguments->requiredDirectoryLength =
        strrchr(rawExecutable, '/') - rawExecutable;

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
        else if (strcmp(currentArg, "version"))
        {
            waterlily_engine_log(INFO,
                                 "Waterlily version " WATERLILY_VERSION ".");
            return false;
        }
        else if (strcmp(currentArg, "license"))
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
    }

    waterlily_engine_log(SUCCESS, "Parsed all provided arguments.");
    return true;
}

