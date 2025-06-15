#include <RPGtk.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    bool success = tk_initialize();
    if (!success) return -1;

    return 0;
}
