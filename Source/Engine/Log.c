#include <WaterlilyRaw.h>
#include <errno.h>
#include <stdarg.h>

void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...)
{
    static const char *const tags[] = {
        [WATERLILY_LOG_TYPE_INFO] = "INFO",
        [WATERLILY_LOG_TYPE_SUCCESS] = " OK ",
        [WATERLILY_LOG_TYPE_WARNING] = "WARN",
        [WATERLILY_LOG_TYPE_ERROR] = "FAIL",
    };

    FILE *output = data->type < WATERLILY_LOG_TYPE_WARNING ? stdout : stderr;
    (void)fprintf(output, "[%s] %-15s ln. %04zu: ", tags[data->type],
                  data->filename, data->line);

    va_list args;
    va_start(args);
    (void)vfprintf(output, format, args);
    (void)fputc('\n', output);
    va_end(args);

    if (data->type >= WATERLILY_LOG_TYPE_WARNING)
        (void)fprintf(output, "\tCurrent ERRNO (may be garbage): %d\n", errno);
}

