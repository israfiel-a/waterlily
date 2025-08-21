#ifndef WATERLILY_INTERNAL_FILES_H
#define WATERLILY_INTERNAL_FILES_H

#include <stdint.h>
#include <stdio.h>

#define WATERLILY_FILE_CONTENTS_CONFIG_MAX_PAIRS 16

typedef enum waterlily_file_type
{
    WATERLILY_GENERIC_FILE,
    WATERLILY_CONFIG_FILE
} waterlily_file_type_t;

typedef union waterlily_file_contents
{
    struct
    {
        struct
        {
            enum
            {
                WATERLILY_CONFIG_FILE_UNKNOWN_KEY,
                WATERLILY_CONFIG_FILE_TITLE_KEY,
                WATERLILY_CONFIG_FILE_SHADERS_KEY
            } key;
            union
            {
                char *title;
                uint32_t **shaders;
            } value;
        } pairs[WATERLILY_FILE_CONTENTS_CONFIG_MAX_PAIRS];
        size_t pairCount;
    } config;
} waterlily_file_contents_t;

#define WATERLILY_RESOURCE_DIRECTORY "./rss/"

bool waterlily_openFile(const char *const name, waterlily_file_type_t type,
                        FILE **file, size_t *size);
static inline void waterlily_closeFile(FILE *file) { fclose(file); }

bool waterlily_interpretFile(FILE *file, size_t size,
                             waterlily_file_type_t type, char *raw,
                             waterlily_file_contents_t *contents);

#endif // WATERLILY_INTERNAL_FILES_H

