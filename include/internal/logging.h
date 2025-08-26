#ifndef WATERLILY_INTERNAL_LOGGING_H
#define WATERLILY_INTERNAL_LOGGING_H

#include <stdint.h>
#define __need_size_t
#include <stddef.h>

typedef enum waterlily_log_type : uint8_t
{
    WATERLILY_LOG_TYPE_INFO,
    WATERLILY_LOG_TYPE_SUCCESS,
    WATERLILY_LOG_TYPE_WARNING,
} waterlily_log_type_t;

typedef struct waterlily_log
{
    const waterlily_log_type_t type;
    const size_t line;
    const char *const filename;
} waterlily_log_t;

void(waterlily_log)(const waterlily_log_t *data, const char *const format, ...);
[[noreturn]]
void(waterlily_report)(const waterlily_log_t *data, const char *const format,
                       ...);

#define waterlily_log(type, format, ...)                                       \
    waterlily_log(                                                             \
        &(waterlily_log_t){WATERLILY_LOG_TYPE_##type, __LINE__, FILENAME},     \
        format __VA_OPT__(, ) __VA_ARGS__)

#define waterlily_report(format, ...)                                          \
    waterlily_report(&(waterlily_log_t){0, __LINE__, FILENAME},                \
                     format __VA_OPT__(, ) __VA_ARGS__)

#endif // WATERLILY_INTERNAL_LOGGING_H

