#include <internal/files.h>
#include <internal/logging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void compileShaders(void)
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

        waterlily_readFile(&file);

        outputFile.text.size = file.text.size + 3;
        char output[outputFile.text.size];
        output[0] = 0;
        strcat(output, file.text.contents);
        output[file.text.size] = 0xA;
        output[file.text.size + 1] = 0xD;
        outputFile.text.contents = output;

        waterlily_writeFile(&outputFile, true);
        waterlily_closeFile(&file);
    }
    waterlily_log(SUCCESS, "Compiled all shaders.");
}

static void getFilepath(char *filepath, waterlily_file_t *file)
{
    static const char *extensions[] = {
        [WATERLILY_TEXT_FILE] = "txt",
        [WATERLILY_CONFIG_FILE] = "config",
        [WATERLILY_FRAGMENT_SHADER_FILE] = "frag",
        [WATERLILY_VERTEX_SHADER_FILE] = "vert",
        [WATERLILY_ARCHIVE_FILE] = "waterlily",
    };

    (void)sprintf(filepath, WATERLILY_ASSET_DIRECTORY "%s.%s", file->name,
                  extensions[file->type]);
    waterlily_log(INFO, "Got full path '%s'.", filepath);
}

static void digestValue(char *key, char *value, char *ch,
                        enum waterlily_config_key keyType,
                        waterlily_file_t *file)
{
    if (value == nullptr)
    {
        *ch = 0;
        waterlily_report("No value given to key '%s'.", key);
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
            waterlily_report("Unimplement configuration key %d.", keyType);
    }
    file->config.pairCount++;
    *ch = 0;
}

static void parseConfig(waterlily_file_t *file)
{
    char *key = (char *)file->text.contents;
    char *value = nullptr;

    enum waterlily_config_key keyType = 0;

    for (char *ch = key; *ch != 0; ++ch)
    {
        if (*ch == ';')
        {
            if (value == nullptr)
                waterlily_report("Comment before value.");

            digestValue(key, value, ch, keyType, file);

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
                waterlily_report("Got double key ('%s').", key);
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
                waterlily_report("Found unknown configuration key '%s'.", key);
            }
            value = ch;
            continue;
        }
        else if (*ch != '\n')
            continue;

        digestValue(key, value, ch, keyType, file);
        value = nullptr;
        key = ch + 1;
    }
}

static void parseShaderArchive(waterlily_file_t *file)
{
    char *currentShader = (char *)file->text.contents;
    char *cursor = currentShader;
    size_t shaderIndex = 0;

    while (*cursor != 0)
    {
        while (*cursor != 0 && *cursor != 0xA && *(cursor + 1) != 0xD)
            cursor++;

        if (*cursor == 0)
            waterlily_report(
                "Malformed shader at index %zu (missing magic end number).",
                shaderIndex);

        *cursor = 0;
        *(cursor + 1) = 0;

        auto shader = &file->shader[shaderIndex];
        shader->size = cursor - currentShader;
        shader->code = (uint32_t *)currentShader;

        cursor += 2;
    }
}

static void parseShaderArchiveSection(char **cursor,
                                      struct waterlily_archive_section *section)
{
    (*cursor)++;
    while (**cursor != 0)
    {
        section->count++;
        section->shaders = realloc(
            &section->shaders,
            sizeof(struct waterlily_archive_shader_section) * section->count);
        section->shaders[section->count - 1].type = **cursor;
        if (**cursor != 0x0 && **cursor != 0x1)
            waterlily_report("Got unknown shader stage '%x'.", **cursor);

        (*cursor)++;
        section->shaders[section->count - 1].size = *(uint16_t *)(*cursor);
        section->shaders[section->count - 1].code = (uint32_t *)strndup(
            *cursor, section->shaders[section->count - 1].size);
        (*cursor) += section->shaders[section->count - 1].size;
    }
}

static void parseAssetArchiveFile(waterlily_file_t *file)
{
    char *cursor = (char *)file->text.contents;
    while (*cursor != 0)
    {
        switch (*cursor)
        {
            case 0x0:
                file->archive.assets.sectionCount++;
                file->archive.assets.sections =
                    realloc(file->archive.assets.sections,
                            sizeof(struct waterlily_archive_section) *
                                file->archive.assets.sectionCount);
                file->archive.assets
                    .sections[file->archive.assets.sectionCount - 1]
                    .type = WATERLILY_SHADER_SECTION;
                parseShaderArchiveSection(
                    &cursor,
                    &file->archive.assets
                         .sections[file->archive.assets.sectionCount - 1]);
                break;
            default:
                waterlily_report("Unknown asset archive section ID '%x'.",
                                 *cursor);
        }
    }
}

static void parseArchiveFile(waterlily_file_t *file)
{
    switch (*file->text.contents)
    {
        case 0x0:
            file->archive.type = WATERLILY_ASSET_ARCHIVE;
            waterlily_log(INFO, "Got asset archive.");
            parseAssetArchiveFile(file);
            break;
        default:
            waterlily_report("Got unknown archive file type '%x'",
                             *file->text.contents);
    }
}

void waterlily_readFile(waterlily_file_t *file)
{
    waterlily_log(INFO, "Opening file '%s' of type %d.", file->name,
                  file->type);

    size_t filepathLength =
        sizeof(WATERLILY_ASSET_DIRECTORY) + strlen(file->name) + 5;
    char filepath[filepathLength];
    getFilepath(filepath, file);

    if (access(filepath, R_OK) != 0)
    {
        if (file->type != WATERLILY_SHADER_FILE)
            compileShaders();
        else
            waterlily_report("File is not accessible.");
    }
    waterlily_log(SUCCESS, "File correctly accessible.");

    FILE *handle = fopen(filepath, "r");
    if (handle == nullptr)
        waterlily_report("Failed to open file.");
    waterlily_log(SUCCESS, "Opened file handle.");

    struct stat stat;
    if (fstat(fileno(handle), &stat) != 0)
        waterlily_report("Failed to stat file.");
    waterlily_log(SUCCESS, "Statted file.");

    char contents[stat.st_size + 1];
    size_t read = fread(contents, 1, stat.st_size, handle);
    if (read != (size_t)stat.st_size)
        waterlily_report("Failed to read file, could only read %zu bytes.",
                         read);
    contents[stat.st_size] = 0;
    waterlily_log(SUCCESS, "Read %zu bytes from file.", read);

    if (fclose(handle) != 0)
        waterlily_report("Failed to close file.");
    waterlily_log(INFO, "Closed file handle.");

    file->text.contents = contents;
    switch (file->type)
    {
        case WATERLILY_TEXT_FILE:
            [[fallthrough]];
        case WATERLILY_VERTEX_SHADER_FILE:
            [[fallthrough]];
        case WATERLILY_FRAGMENT_SHADER_FILE:
            file->text.contents = strdup(contents);
            file->text.size = stat.st_size;
            break;
        case WATERLILY_SHADER_FILE:
            parseShaderArchive(file);
            break;
        case WATERLILY_CONFIG_FILE:
            parseConfig(file);
            break;
        case WATERLILY_ARCHIVE_FILE:
            parseArchiveFile(file);
            break;
    }
}

void waterlily_writeFile(waterlily_file_t *file, bool append)
{
    waterlily_log(INFO, "Writing to file '%s' of type %d.", file->name,
                  file->type);

    size_t filepathLength =
        sizeof(WATERLILY_ASSET_DIRECTORY) + strlen(file->name) + 5;
    char filepath[filepathLength];
    getFilepath(filepath, file);

    FILE *handle = fopen(filepath, (append ? "a" : "w"));
    if (handle == nullptr)
        waterlily_report("Failed to open file.");
    waterlily_log(SUCCESS, "Opened file handle.");

    size_t wrote = fwrite(file->text.contents, 1, file->text.size, handle);
    if (wrote != file->text.size)
        waterlily_report("Failed to write to file, only wrote %zu bytes",
                         wrote);
    waterlily_log(SUCCESS, "Wrote %zu bytes to file.", wrote);

    if (fclose(handle) != 0)
        waterlily_report("Failed to close file.");
    waterlily_log(INFO, "Closed file handle.");
}

void waterlily_closeFile(waterlily_file_t *file) { (void)file; }

