#include <errno.h>
#include <internal/logging.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <waterlily.h>

#define STRINGIFY2(expr) #expr
#define STRINGIFY(expr) STRINGIFY2(expr)

void(waterlily_log)(const waterlily_log_t *data, const char *const format, ...)
{
    static const char *const tags[] = {
        [WATERLILY_LOG_TYPE_INFO] = "INFO",
        [WATERLILY_LOG_TYPE_SUCCESS] = " OK ",
        [WATERLILY_LOG_TYPE_WARNING] = "WARN",
    };

    FILE *output = data->type < WATERLILY_LOG_TYPE_WARNING ? stdout : stderr;
    (void)fprintf(output, "[%s] %-15s ln. %04zu: ", tags[data->type],
                  data->filename, data->line);

    va_list args;
    va_start(args);
    (void)vfprintf(output, format, args);
    (void)fputc('\n', output);
    va_end(args);
}

void(waterlily_report)(const waterlily_log_t *data, const char *const format,
                       ...)
{
    (void)fprintf(stderr, "[FAIL] %-15s ln. %04zu: ", data->filename,
                  data->line);

    va_list args;
    va_start(args);
    (void)vfprintf(stderr, format, args);
    va_end(args);
    (void)fprintf(stderr, "\n       Current ERRNO (may be garbage): %d\n",
                  errno);
    exit(-1);
}

