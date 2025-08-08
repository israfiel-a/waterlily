#include <Waterlily.h>
#include <linux/limits.h>
#include <string.h>
#include <unistd.h>

bool waterlily_engine_setup(waterlily_context_t *context)
{
    char wd[PATH_MAX];
    (void)strncpy(wd, context->arguments.requiredDirectory,
                  context->arguments.requiredDirectoryLength);
    wd[context->arguments.requiredDirectoryLength] = 0;

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

