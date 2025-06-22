#include <Waterlily.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    bool success = waterlily_initialize("Example");
    if (!success) return -1;

    waterlily_run();

    waterlily_cleanup();
    return 0;
}
