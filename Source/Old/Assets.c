#include <Waterlily.h>
#include <sys/stat.h>
#include <sys/wait.h>

/**
 * @var const char *const waterlily_shaderSourcePath
 * @brief The path to the local shader source directory, kept separate from
 * the rest of the paths for deduplication reasons.
 * @since v0.0.0.13
 */
static const char *const waterlily_shaderSourcePath = "Shaders/Source/";

/**
 * @var const char *const waterlily_shaderCompiledPath
 * @brief The path to the local shader compiled output directory, kept separate
 * from the rest of the paths for deduplication reasons.
 * @since v0.0.0.13
 */
static const char *const waterlily_shaderCompiledPath = "Shaders/Compiled/";

/**
 * @var const char *const waterlily_infos[waterlily_TYPE_COUNT][2]
 * @brief A list of subdirectory and extensions for each file type recognized by
 * the library.
 * @since v0.0.0.18
 */
static const char *const waterlily_infos[waterlily_TYPE_COUNT][2] = {
    [waterlily_TEXT] = {nullptr, ".txt"},
    [waterlily_GLSL_VERTEX] = {waterlily_shaderSourcePath, ".vert"},
    [waterlily_GLSL_FRAGMENT] = {waterlily_shaderSourcePath, ".frag"},
    [waterlily_SPIRV_VERTEX] = {waterlily_shaderCompiledPath, "-vert.spv"},
    [waterlily_SPIRV_FRAGMENT] = {waterlily_shaderCompiledPath, "-frag.spv"},
    [waterlily_SYSTEM] = {nullptr, nullptr},
};

bool waterlily_file_access(const waterlily_file_t *const file)
{
    char path[waterlily_MAX_PATH_LENGTH];
    waterlily_file_getPath(file, path);
    return access(path, F_OK) == 0;
}

bool waterlily_closeFile(const waterlily_file_t *const file)
{
    if (__builtin_expect(fclose(file->handle) != 0, 0))
    {
        waterlily_log(ERROR, "Failed to close file '%s'.", file->basename);
        return false;
    }
    waterlily_log(SUCCESS, "Closed file '%s'.", file->basename);
    return true;
}

/**
 * @fn void waterlily_strncat(char *dest, const char *const src, size_t
 * *consumed)
 * @brief A custom @c strncat implementation to fit the specific needs of
 * filepath generation within the open/close/load/execute functions of the
 * library.
 * @since v0.0.0.17
 *
 * @param[out] dest The buffer into which to copy the string data.
 * @param[in] src The data to be copied into the destination buffer.
 * @param[in, out] consumed The amount of characters that have been consumed
 * within @c dest already.
 */
[[gnu::nonnull(1, 3)]]
void waterlily_strncat(char *dest, const char *const src, size_t *consumed)
{
    if (src == nullptr) return;

    char *tempSource = (char *)src;
    const size_t n = waterlily_MAX_PATH_LENGTH - *consumed;
    dest += *consumed;
    while (*tempSource != 0 && *consumed < n)
    {
        *dest++ = *tempSource;
        tempSource++;
        (*consumed)++;
    }
}

void waterlily_createFilepath(const waterlily_file_t *const file, char *path)
{
    size_t consumed = 0;
    if (__builtin_expect(file->type == waterlily_SYSTEM, 0))
    {
        // It's more efficient to just do this inline.
        char *baseDirectory = waterlily_SYSTEM_DIRECTORY;
        for (size_t i = 0; i < waterlily_SYSTEM_DIRECTORY_LENGTH; i++)
            path[i] = baseDirectory[i];
        consumed = waterlily_SYSTEM_DIRECTORY_LENGTH;
        waterlily_strncat(path, file->basename, &consumed);
    }
    else
    {
        // It's more efficient to just do this inline.
        char *baseDirectory = waterlily_BASE_DIRECTORY;
        for (size_t i = 0; i < waterlily_BASE_DIRECTORY_LENGTH; i++)
            path[i] = baseDirectory[i];
        consumed = waterlily_BASE_DIRECTORY_LENGTH;

        auto info = waterlily_infos[file->type];
        waterlily_strncat(path, info[0], &consumed);
        waterlily_strncat(path, file->basename, &consumed);
        waterlily_strncat(path, info[1], &consumed);
    }
}

bool waterlily_openFile(waterlily_file_t *file,
                       waterlily_permissions_t permissions)
{
    char path[waterlily_MAX_PATH_LENGTH];
    waterlily_createFilepath(file, path);

    char *mode;
    switch (permissions)
    {
        case waterlily_READ:      mode = "r"; break;
        case waterlily_WRITE:     mode = "w"; break;
        case waterlily_APPEND:    mode = "a"; break;
        case waterlily_READWRITE: mode = "w+"; break;
        default:                 mode = "a+"; break;
    }

    file->handle = fopen(path, mode);
    if (__builtin_expect(file->handle == nullptr, 0))
    {
        waterlily_log(ERROR, "Failed to open file '%s'.", path);
        return false;
    }
    waterlily_log(SUCCESS, "Opened file '%s'.", path);
    return true;
}

bool waterlily_getFileSize(waterlily_file_t *file)
{
    struct stat stats;
    // Nearly 10x faster than fseek/ftell in my tests.
    if (__builtin_expect(fstat(fileno(file->handle), &stats) == -1, 0))
    {
        waterlily_log(ERROR, "Failed to stat file '%s'.", file->basename);
        return false;
    }
    file->size = stats.st_size;
    waterlily_log(SUCCESS, "Got size of file '%s': %zu.", file->basename,
                 file->size);
    return true;
}

bool waterlily_loadFile(const waterlily_file_t *const file, char *contents)
{
    if (__builtin_expect(
            fread(contents, 1, file->size, file->handle) != file->size, 0))
    {
        waterlily_log(ERROR, "Failed to properly read file '%s'.",
                     file->basename);
        return false;
    }
    waterlily_log(SUCCESS, "Loaded %zu bytes of file '%s'.", file->size,
                 file->basename);
    return true;
}

bool waterlily_writeFile(const waterlily_file_t *const file,
                        const char *const contents)
{
    if (__builtin_expect(
            fwrite(contents, 1, file->size, file->handle) != file->size, 0))
    {
        waterlily_log(ERROR, "Failed to write to file '%s'.", file->basename);
        return false;
    }
    waterlily_log(SUCCESS, "Wrote %zu bytes to file '%s'.", file->size,
                 file->basename);
    return true;
}

bool waterlily_executeFile(const waterlily_file_t *const file,
                          const char *const *const argv, size_t argc,
                          int *status)
{
    char path[waterlily_MAX_PATH_LENGTH];
    waterlily_createFilepath(file, path);

    if (__builtin_expect(access(path, X_OK) == -1, 0))
    {
        waterlily_log(ERROR, "Cannot execute file '%s'.", path);
        return false;
    }

    pid_t pid = fork();
    if (__builtin_expect(pid == -1, 0))
    {
        waterlily_log(ERROR, "Failed to fork process.");
        return false;
    }

    char *trueArgv[argc + 2];
    trueArgv[0] = path;
    for (size_t i = 0; i < argc; i++) trueArgv[i + 1] = (char *)argv[i];
    trueArgv[argc + 1] = nullptr;

    // This is executed within the new process.
    if (__builtin_expect(
            pid == 0 && execve(path, (char *const *)trueArgv, nullptr) == -1,
            0))
    {
        waterlily_log(ERROR, "Failed to execute file '%s'.", path);
        return false;
    }

    int processStatus = 0;
    pid_t waitStatus = waitpid(pid, &processStatus, 0);
    if (__builtin_expect(waitStatus == (pid_t)-1, 0))
    {
        waterlily_log(ERROR, "Execution of '%s' interrupted by a signal.",
                     path);
        return false;
    }

    // This could very likely happen or not happen. Don't expect anything.
    if (!WIFEXITED(processStatus))
    {
        waterlily_log(ERROR,
                     "File '%s' ended execution with an unexpected result.",
                     path);
        return false;
    }
    *status = WEXITSTATUS(processStatus);
    waterlily_log(SUCCESS, "Executed file '%s'. Exited with status code %d.",
                 file->basename, *status);
    return true;
}

bool waterlily_glslToSPIRV(const waterlily_file_t *const file)
{
    char path[waterlily_MAX_PATH_LENGTH];
    waterlily_createFilepath(file, path);

    char outputPath[waterlily_MAX_PATH_LENGTH];
    waterlily_file_t outputFile = *file;
    outputFile.type =
        (file->type == waterlily_GLSL_FRAGMENT ? waterlily_SPIRV_FRAGMENT
                                              : waterlily_SPIRV_VERTEX);
    waterlily_createFilepath(&outputFile, outputPath);

    const char *const argv[] = {
        "--target-env",   "vulkan1.3", "-e",          "main",  "-g0",     "-t",
        "--glsl-version", "460",       "--spirv-val", "--lto", "--quiet", "-o",
        outputPath,       path,
    };

    int status = 0;
    waterlily_file_t glslangFile = {.basename = "glslang",
                                   .type = waterlily_SYSTEM};
    if (!waterlily_executeFile(&glslangFile, argv, 14, &status))
    {
        waterlily_log(ERROR, "Couldn't compile shader '%s'. Code %d.", path,
                     status);
        return false;
    }
    waterlily_log(SUCCESS, "Compiled shader '%s'.", path);
    return true;
}

void waterlily_splitStem(const char *const original, char *filename,
                        char *extension)
{
    char *originalCopy = (char *)original;
    size_t written = 0;
    while (*originalCopy != '.' && written < waterlily_MAX_PATH_LENGTH)
    {
        *filename = *originalCopy;
        filename++;
        originalCopy++;
        written++;
    }
    filename[written] = 0;

    while (*originalCopy++ != 0)
    {
        *extension = *originalCopy;
        extension++;
    }
}

bool waterlily_changeDirectory(const char *const path)
{
    size_t lastSlash = 0;

    char *pathCopy = (char *)path;

    while (*pathCopy != 0)
    {
        if (*pathCopy == '/') lastSlash = pathCopy - path;
        pathCopy++;
    }

    char *actualPath = (char *)path;
    if (lastSlash != 0) actualPath[lastSlash] = 0;

    if (chdir(actualPath) != 0)
    {
        waterlily_log(ERROR, "Failed to change directory to '%s' ('%s').", path,
                     actualPath);
        return false;
    }
    waterlily_log(SUCCESS, "Changed path to '%s'.", actualPath);
    return true;
}

