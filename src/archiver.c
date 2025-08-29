#include <internal/logging.h>
#include <archiver/compressor.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    (void)argc;
    waterlily_log(INFO, "Starting compression process.");

    char *path = argv[0];
    path[strrchr(path, '/') - argv[0]] = 0;

    int status = chdir(path);
    if (status < 0)
    {
        waterlily_report("Failed to move working directory into '%s'.", path);
        return -1;
    }
    waterlily_log(SUCCESS, "Changed working directory to '%s'.", path);

    waterlily_flattenAssets();

    return 0;
}

