#include "internal/files.h"

#include "internal.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static bool compileShaders(void)
{
    const char *const stageNames[] = {
        "vertex",
        "fragment",
    };

    waterlily_file_t outputFile = {
        .type = WATERLILY_SHADER_FILE,
        .name = "shaders",
    };

    for (size_t i = 0; i < WATERLILY_SHADER_STAGES; ++i)
    {
        const char *const stage = stageNames[i];
        size_t currentPathLength =
            sizeof(WATERLILY_SHADER_DIRECTORY) + strlen(stage);
        char currentPath[currentPathLength];
        (void)sprintf(currentPath, WATERLILY_SHADER_DIRECTORY "%s", stage);

        waterlily_file_t file = {
            .type = WATERLILY_TEXT_FILE,
            .name = currentPath,
        };

        if (!waterlily_readFile(&file))
        {
            waterlily_engine_log(
                ERROR, "Failed to open required shader stage '%s'.", stage);
            return false;
        }

        outputFile.text.size = file.text.size + 2;
        char output[outputFile.text.size];
        strcat(output, file.text.contents);
        output[file.text.size] = 0xA;
        output[file.text.size + 1] = 0xD;
        outputFile.text.contents = output;

        if (!waterlily_writeFile(&outputFile, true))
            return false;

        waterlily_closeFile(&file);
    }

    waterlily_engine_log(SUCCESS, "Compiled all shaders.");
    return true;
}

static void getFilepath(char *filepath, waterlily_file_t *file)
{
    static const char extensions[] = {
        [WATERLILY_TEXT_FILE] = 't',
        [WATERLILY_CONFIG_FILE] = 'c',
        [WATERLILY_SHADER_FILE] = 's',
    };

    (void)sprintf(filepath, WATERLILY_ASSET_DIRECTORY "%s.wl%c", file->name,
                  extensions[file->type]);
    waterlily_engine_log(INFO, "Got full path '%s'.", filepath);
}

static bool digestValue(char *key, char *value, char *ch,
                        enum waterlily_config_key keyType,
                        waterlily_file_t *file)
{
    if (value == nullptr)
    {
        *ch = 0;
        waterlily_engine_log(ERROR, "No value given to key '%s'.", key);
        return false;
    }

    switch (keyType)
    {
        case WATERLILY_CONFIG_TITLE_KEY:
            file->config.pairs[file->config.pairCount] =
                (typeof(file->config.pairs[0])){
                    .key = keyType,
                    .value.title = strndup(value, ch - value),
                };
            break;
        case WATERLILY_CONFIG_AUTHOR_KEY:
            file->config.pairs[file->config.pairCount] =
                (typeof(file->config.pairs[0])){
                    .key = keyType,
                    .value.author = strndup(value, ch - value),
                };
            break;
        case WATERLILY_CONFIG_VERSION_KEY:
            file->config.pairs[file->config.pairCount] =
                (typeof(file->config.pairs[0])){
                    .key = keyType,
                    .value.version = strndup(value, ch - value),
                };
            break;
        default:
            waterlily_engine_log(ERROR, "Unimplement configuration key %d.",
                                 keyType);
            return false;
    }
    file->config.pairCount++;

    *ch = 0;
    waterlily_engine_log(ERROR, "%s", key);

    return true;
}

static bool parseConfig(waterlily_file_t *file)
{
    char *key = (char *)file->text.contents;
    char *value = nullptr;

    enum waterlily_config_key keyType = 0;

    for (char *ch = key; *ch != 0; ++ch)
    {
        if (*ch == ';')
        {
            if (value == nullptr)
            {
                waterlily_engine_log(ERROR, "Comment before value.");
                return false;
            }

            if (!digestValue(key, value, ch, keyType, file))
                return false;

            while (*ch != '\n' && *ch != '0')
                ch++;
            if (*ch == 0)
                break;

            value = nullptr;
            key = ch + 1;
            continue;
        }

        if (*ch != '\n' && *ch == '=')
        {
            if (value != nullptr)
            {
                *ch = 0;
                waterlily_engine_log(ERROR, "Got double key ('%s').", key);
                return false;
            }

            if (strncmp(key, "title", 5) == 0)
                keyType = WATERLILY_CONFIG_TITLE_KEY;
            else if (strncmp(key, "author", 6) == 0)
                keyType = WATERLILY_CONFIG_AUTHOR_KEY;
            else if (strncmp(key, "version", 7) == 0)
                keyType = WATERLILY_CONFIG_VERSION_KEY;
            else
            {
                *ch = 0;
                waterlily_engine_log(
                    ERROR, "Found unknown configuration key '%s'.", key);
                return false;
            }
            value = ch;
            continue;
        }
        else if (*ch != '\n')
            continue;

        if (!digestValue(key, value, ch, keyType, file))
            return false;
        value = nullptr;
        key = ch + 1;
    }

    return true;
}

bool waterlily_readFile(waterlily_file_t *file)
{
    waterlily_engine_log(INFO, "Opening file '%s' of type %d.", file->name,
                         file->type);

    size_t filepathLength = strlen(file->name) + 5;
    char filepath[filepathLength];
    getFilepath(filepath, file);

    if (access(filepath, R_OK) != 0)
    {
        waterlily_engine_log(ERROR, "File is not accessible.");
        if (file->type != WATERLILY_SHADER_FILE || !compileShaders())
            return false;
        waterlily_engine_log(ERROR, "%d:%s", access(filepath, R_OK), filepath);
    }
    waterlily_engine_log(SUCCESS, "File correctly accessible.");

    FILE *handle = fopen(filepath, "r");
    if (handle == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open file.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Opened file handle.");

    struct stat stat;
    if (fstat(fileno(handle), &stat) != 0)
    {
        waterlily_engine_log(ERROR, "Failed to stat file.");

        if (fclose(handle) != 0)
            waterlily_engine_log(ERROR, "Failed to close file.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Statted file.");

    char contents[stat.st_size + 1];
    size_t read = fread(contents, 1, stat.st_size, handle);
    if (read != (size_t)stat.st_size)
    {
        waterlily_engine_log(
            ERROR, "Failed to read file, could only read %zu bytes.", read);

        if (fclose(handle) != 0)
            waterlily_engine_log(ERROR, "Failed to close file.");
        return false;
    }
    contents[stat.st_size] = 0;
    waterlily_engine_log(SUCCESS, "Read %zu bytes from file.", read);

    if (fclose(handle) != 0)
    {
        waterlily_engine_log(ERROR, "Failed to close file.");
        return false;
    }
    waterlily_engine_log(INFO, "Closed file handle.");

    switch (file->type)
    {
        case WATERLILY_TEXT_FILE:
            file->text.contents = strdup(contents);
            file->text.size = stat.st_size;
            break;
        case WATERLILY_SHADER_FILE:
            waterlily_engine_log(ERROR, "unimplemented");
            return false;
        case WATERLILY_CONFIG_FILE:
            file->text.contents = contents;
            if (!parseConfig(file))
                return false;
    }

    return true;
}

bool waterlily_writeFile(waterlily_file_t *file, bool append)
{
    waterlily_engine_log(INFO, "Writing to file '%s' of type %d.", file->name,
                         file->type);

    size_t filepathLength = strlen(file->name) + 5;
    char filepath[filepathLength];
    getFilepath(filepath, file);

    FILE *handle = fopen(filepath, (append ? "a" : "w"));
    if (handle == nullptr)
    {
        waterlily_engine_log(ERROR, "Failed to open file.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Opened file handle.");

    size_t wrote = fwrite(file->text.contents, 1, file->text.size, handle);
    if (wrote != file->text.size)
    {
        waterlily_engine_log(
            ERROR, "Failed to write to file, only wrote %zu bytes", wrote);

        if (fclose(handle) != 0)
            waterlily_engine_log(ERROR, "Failed to close file.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Wrote %zu bytes to file.", wrote);

    if (fclose(handle) != 0)
    {
        waterlily_engine_log(ERROR, "Failed to close file.");
        return false;
    }
    waterlily_engine_log(INFO, "Closed file handle.");

    return true;
}

void waterlily_closeFile(waterlily_file_t *file) { (void)file; }

