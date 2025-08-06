#include <Waterlily.h>
#include <linux/limits.h>
#include <string.h>
#include <unistd.h>

bool waterlily_engine_setup(const waterlily_arguments_t *const arguments)
{
    char wd[PATH_MAX];
    (void)strncpy(wd, arguments->requiredDirectory,
                  arguments->requiredDirectoryLength);
    wd[arguments->requiredDirectoryLength] = 0;

    int status = chdir(wd);
    if (status < 0)
    {
        waterlily_engine_log(ERROR,
                             "Failed to move working directory into '%s'.", wd);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Changed working directory to '%s'.", wd);

    return true;
}

