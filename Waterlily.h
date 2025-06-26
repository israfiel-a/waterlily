#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <stdint.h>

bool waterlily_initialize(const char *title, uint32_t version);

void waterlily_run(void);

void waterlily_cleanup(void);

#endif // WATERLILY_MAIN_H
