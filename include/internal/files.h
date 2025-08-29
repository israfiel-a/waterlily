#ifndef WATERLILY_INTERNAL_FILES_H
#define WATERLILY_INTERNAL_FILES_H

#include <stdint.h>
#define __need_size_t
#include <stddef.h>

#define WATERLILY_SHADER_STAGES 2
#define WATERLILY_MAX_CONFIG_PAIRS 16

#define WATERLILY_ASSET_DIRECTORY "./rss/"
#define WATERLILY_SHADER_DIRECTORY "shaders/"

typedef struct waterlily_file
{
    char *name;
    enum
    {
        WATERLILY_TEXT_FILE,
        WATERLILY_CONFIG_FILE,
        WATERLILY_FRAGMENT_SHADER_FILE,
        WATERLILY_VERTEX_SHADER_FILE,
        WATERLILY_SHADER_FILE,
        WATERLILY_ARCHIVE_FILE
    } type;
    union
    {
        struct
        {
            size_t size;
            char *contents;
        } text;
        struct
        {
            struct
            {
                enum waterlily_config_key
                {
                    WATERLILY_CONFIG_TITLE_KEY,
                    WATERLILY_CONFIG_AUTHOR_KEY,
                    WATERLILY_CONFIG_VERSION_KEY,
                } key;
                union
                {
                    char *title;
                    char *author;
                    char *version;
                } value;
            } pairs[WATERLILY_MAX_CONFIG_PAIRS];
            size_t pairCount;
        } config;
        struct
        {
            uint32_t *code;
            size_t size;
        } shader[WATERLILY_SHADER_STAGES];
    };
} waterlily_file_t;

void waterlily_readFile(waterlily_file_t *file);
void waterlily_writeFile(waterlily_file_t *file, bool append);
void waterlily_closeFile(waterlily_file_t *file);

#endif // WATERLILY_INTERNAL_FILES_H

