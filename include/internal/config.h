#ifndef WATERLILY_INTERNAL_CONFIG_H
#define WATERLILY_INTERNAL_CONFIG_H

struct waterlily_configuration
{
    char *title;
    char *author;
    char *version;
    struct
    {
        bool displayFPS : 1;
    } arguments;
};

struct waterlily_configuration *
waterlily_initializeConfiguration(int argc, const char *const *const argv);

#endif // WATERLILY_INTERNAL_CONFIG_H

