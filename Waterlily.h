#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <stdint.h>

bool waterlily_initialize(int argc, const char* const *const argv);

void waterlily_run(void);

void waterlily_cleanup(void);

#endif // WATERLILY_MAIN_H
