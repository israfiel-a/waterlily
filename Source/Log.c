#include <Waterlily.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const char *const tags[] = {
    [WATERLILY_INFO_LOG_TYPE] = "INFO",
    [WATERLILY_SUCCESS_LOG_TYPE] = " OK ",
    [WATERLILY_ERROR_LOG_TYPE] = "FAIL",
};

static const uint8_t colors[] = {
    [WATERLILY_INFO_LOG_TYPE] = 0,
    [WATERLILY_SUCCESS_LOG_TYPE] = 92,
    [WATERLILY_ERROR_LOG_TYPE] = 91,
};

static int columnSize = 0;

void(waterlily_log)(waterlily_file_data_t data, waterlily_log_type_t type,
                    FILE *redirect, const char *const format, ...)
{
    if (columnSize == 0)
    {
        struct winsize w_stdout, w_stderr;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w_stdout);
        ioctl(STDERR_FILENO, TIOCGWINSZ, &w_stderr);
        columnSize = (w_stdout.ws_col < w_stderr.ws_col ? w_stdout.ws_col
                                                        : w_stderr.ws_col);
    }

    if (redirect == nullptr)
        redirect = type == WATERLILY_ERROR_LOG_TYPE ? stderr : stdout;

    char message[columnSize];
    snprintf(message, columnSize,
             "[\033[%dm%s\033[0m] %-13s fn. %-20s ln. %04zu: %s \n",
             colors[type], tags[type], data.file, data.function, data.line,
             format);
    if (type == WATERLILY_ERROR_LOG_TYPE)
    {
        char submessage[columnSize];
        snprintf(submessage, columnSize,
                 "\t\tCurrent ERRNO (may be garbage): %d\n", errno);
        strncat(message, submessage, columnSize);
    }

    va_list args;
    va_start(args, format);
    vfprintf(redirect, message, args);
    va_end(args);
}

