#include <RPGtk.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    bool success = tk_initialize("Example");
    if (!success) return -1;

    return 0;
}
