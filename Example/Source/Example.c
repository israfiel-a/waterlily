#include <RPGtk.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    bool success = rpgtk_initialize("Example");
    if (!success) return -1;

    rpgtk_run();

    rpgtk_cleanup();
    return 0;
}
