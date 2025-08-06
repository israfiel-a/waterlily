#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef enum waterlily_log_type : uint8_t
{
    WATERLILY_INFO_LOG_TYPE,
    WATERLILY_SUCCESS_LOG_TYPE,
    WATERLILY_ERROR_LOG_TYPE
} waterlily_log_type_t;

typedef struct waterlily_file_data
{
    const char *const file;
    const char *const function;
    const size_t line;
} waterlily_file_data_t;

bool waterlily_initialize(int argc, const char *const *const argv);
void waterlily_run(void);
void waterlily_cleanup(void);

#if BUILD_TYPE == 0

void(waterlily_log)(waterlily_file_data_t data, waterlily_log_type_t type,
                    FILE *redirect, const char *const format, ...);

#define waterlily_log(type, message, ...)                                      \
    waterlily_log((waterlily_file_data_t){FILENAME, __func__, __LINE__},       \
                  WATERLILY_##type##_LOG_TYPE, nullptr,                        \
                  message __VA_OPT__(, ) __VA_ARGS__)

#else
#define waterlily_log(type, message, ...)
#endif

/**
 * @def waterlily_BASE_DIRECTORY
 * @brief The base directory for project assets. This is not where external
 * files are looked for, for that see @ref waterlily_SYSTEM_DIRECTORY.
 * @since v0.0.0.13
 */
#define waterlily_BASE_DIRECTORY "./Assets/"

/**
 * @def waterlily_BASE_DIRECTORY_LENGTH
 * @brief The length in characters of the base asset directory filepath,
 * excluding the null terminator.
 * @since v0.0.0.17
 */
#define waterlily_BASE_DIRECTORY_LENGTH 9

// Allow the user/application to define their own system directory.
#ifndef waterlily_SYSTEM_DIRECTORY
/**
 * @def waterlily_SYSTEM_DIRECTORY
 * @brief The system directory for external assets like executables and
 * libraries. This is not where internal asset files are looked for, for that
 * see @ref waterlily_BASE_DIRECTORY.
 * @since v0.0.0.14
 */
#define waterlily_SYSTEM_DIRECTORY "/usr/bin/"
#endif

/**
 * @def waterlily_SYSTEM_DIRECTORY_LENGTH
 * @brief The length in characters of the system asset directory filepath,
 * excluding the null terminator.
 * @since v0.0.0.19
 */
#define waterlily_SYSTEM_DIRECTORY_LENGTH sizeof(waterlily_SYSTEM_DIRECTORY) - 1

/**
 * @def waterlily_MAX_PATH_LENGTH
 * @brief The max length in characters (plus the null terminator) a path can be
 * in order to be properly handled by the library. This is defined for
 * efficiency and performance reasons.
 * @since v0.0.0.13
 */
#define waterlily_MAX_PATH_LENGTH 128

/**
 * @enum waterlily_permissions
 * @brief The various permissions that a file may be opened under. This is not a
 * bitmask, each enum value corresponds to a different logic chain which is
 * entirely exclusive from the others.
 * @since v0.0.0.13
 *
 * @showenumvalues
 */
typedef enum waterlily_permissions
{
    /**
     * @var waterlily_permissions waterlily_READ
     * @brief Open the file in read-only mode.
     * @since v0.0.0.13
     */
    waterlily_READ,
    /**
     * @var waterlily_permissions waterlily_WRITE
     * @brief Open the file in writing mode, with the file pointer at the
     * beginning of the file. If the file doesn't exist, it will be created.
     * @since v0.0.0.13
     */
    waterlily_WRITE,
    /**
     * @var waterlily_permissions waterlily_APPEND
     * @brief Open the file in writing mode, with the file pointer at the end of
     * the file. If the file doesn't exist, it will be created.
     * @since v0.0.0.13
     */
    waterlily_APPEND,
    /**
     * @var waterlily_permissions waterlily_READWRITE
     * @brief Open the file in reading and writing mode simultaneously, with the
     * file pointer at the beginning of the file. If the file doesn't exist, it
     * will be created.
     * @since v0.0.0.13
     */
    waterlily_READWRITE,
    /**
     * @var waterlily_permissions waterlily_READAPPEND
     * @brief Open the file in reading and writing mode simultaneously, with the
     * file pointer at the end of the file. If the file doesn't exist, it will
     * be created.
     * @since v0.0.0.15
     */
    waterlily_READAPPEND,
} waterlily_permissions_t;

/**
 * @enum waterlily_type
 * @brief The various types of files that have specific handling cases within
 * the library. This is not a bitmask.
 * @since v0.0.0.12
 *
 * @showenumvalues
 */
typedef enum waterlily_type
{
    /**
     * @var waterlily_type waterlily_TEXT
     * @brief A text file. This has no special handling protocols; it's treated
     * as a string of bytes. It comes with the extension ".txt".
     * @since v0.0.0.1
     */
    waterlily_TEXT,
    /**
     * @var waterlily_type waterlily_GLSL_VERTEX
     * @brief A GLSL vertex shader file. This is one of the possible inputs for
     * the @ref waterlily_glslToSPIRV function. It comes with the extension
     * ".vert".
     * @since v0.0.0.2
     */
    waterlily_GLSL_VERTEX,
    /**
     * @var waterlily_type waterlily_GLSL_FRAGMENT
     * @brief A GLSL fragment shader file. This is one of the possible inputs
     * for the @ref waterlily_glslToSPIRV function. It comes with the extension
     * ".frag".
     * @since v0.0.0.2
     */
    waterlily_GLSL_FRAGMENT,
    /**
     * @var waterlily_type waterlily_SPIRV_VERTEX
     * @brief A SPIRV vertex shader file. This is treated as a string of bytes,
     * and is the output for the @ref waterlily_glslToSPIRV function. It comes
     * with the extension "-vert.spv".
     * @since v0.0.0.13
     */
    waterlily_SPIRV_VERTEX,
    /**
     * @var waterlily_type waterlily_SPIRV_FRAGMENT
     * @brief A SPIRV fragment shader file. This is treated as a string of
     * bytes, and is the output for the @ref waterlily_glslToSPIRV function. It
     * comes with the extension "-frag.spv".
     * @since v0.0.0.13
     */
    waterlily_SPIRV_FRAGMENT,
    /**
     * @var waterlily_type waterlily_SYSTEM
     * @brief A system executable file. If attempted to be loaded, this is
     * handled as a string of bytes. This filetype can be used as an input for
     * the @ref waterlily_executeFile function. This comes with no file
     * extension.
     * @since v0.0.0.14
     */
    waterlily_SYSTEM,
} waterlily_type_t;

/**
 * @def waterlily_TYPE_COUNT
 * @brief The count of recognized filetypes by the library.
 * @since v0.0.0.18
 */
#define waterlily_TYPE_COUNT 6

/**
 * @struct waterlily_file waterlily.h "Ageratum.h"
 * @brief The core file structure for the library. This contains all important
 * information about a generic file, including its type, filename, handle, and
 * size.
 * @since v0.0.0.1
 */
typedef struct waterlily_file
{
    /**
     * @property basename
     * @brief The basename of the file.
     * @since v0.0.0.1
     */
    char *basename;
    /**
     * @property type
     * @brief The type of the file.
     * @since v0.0.0.13
     */
    waterlily_type_t type;
    /**
     * @property handle
     * @brief The underlying handle of the file within the filesystem.
     * @since v0.0.0.13
     */
    FILE *handle;
    /**
     * @property size
     * @brief The size of the file in bytes.
     * @since v0.0.0.1
     */
    size_t size;
} waterlily_file_t;

/**
 * @fn bool waterlily_openFile(waterlily_file_t *file, ageratum_permissions_t
 * permissions)
 * @brief Open a file and store its handle in the provided structure. The file
 * must have its filename and filetype set, all other values will be ignored.
 * @since v0.0.0.13
 *
 * @remark Should a file handle have previously have been in this file
 * structure, it should be closed before calling, as it will be leaked on
 * overwrite.
 *
 * @param[in, out] file The file structure we are going to be operating on and
 * storing to.
 * @param[in] permissions The permissions to open the file under.
 *
 * @return A boolean value representing whether or not the file was opened
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function usually fails because the given file
 * does not exist.
 */
[[gnu::nonnull(1)]] [[gnu::hot]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_openFile(waterlily_file_t *file,
                        waterlily_permissions_t permissions);

/**
 * @fn bool waterlily_closeFile(waterlily_file_t *file)
 * @brief Close a file handle.
 * @since v0.0.0.13
 *
 * @param[in] file The file structure to be operated on. The file handle is
 * garbage after this function's completion.
 *
 * @return A boolean value representing whether or not the file was closed
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function usually fails because of IO errors when
 * flushing the file's buffer.
 */
[[gnu::nonnull(1)]] [[gnu::hot]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_closeFile(const waterlily_file_t *const file);

/**
 * @fn bool waterlily_loadFile(waterlily_file_t *file, char *contents)
 * @brief Load all the contents of the given file from disk. The given file
 * structure must have a valid file pointer and its size must have been polled
 * via @ref waterlily_getFileSize.
 * @since v0.0.0.1
 *
 * @remark This function does not add a terminating NUL character to the loaded
 * bytes.
 *
 * @param[in] file The file structure to be operated on.
 * @param[out] contents An array of bytes in which the file's contents will be
 * inserted. This must be large enough to store the file's entire contents.
 *
 * @return A boolean value representing whether or not the file was loaded
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function typically fails because of IO errors.
 */
[[gnu::nonnull(1, 2)]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_loadFile(const waterlily_file_t *const file, char *contents);

/**
 * @fn bool waterlily_writeFile(const waterlily_file_t *const file, const char
 * *const contents)
 * @brief Write the given contents to the given file. The given file must
 * contain a valid file handle, and the given file size is the count of elements
 * that are to be written.
 * @since v0.0.0.1
 *
 * @param[in] file The file structure to be operated on.
 * @param[out] contents An array of bytes which will be written to the file.
 * This must be the same length or longer than described in the provided file
 * structure's @c size property.
 *
 * @return A boolean value representing whether or not the file was written to
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function typically fails because of IO errors.
 */
[[gnu::nonnull(1, 2)]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_writeFile(const waterlily_file_t *const file,
                         const char *const contents);

/**
 * @fn bool waterlily_getFileSize(waterlily_file_t *file)
 * @brief Get the size of the given file in bytes. The given file's handle must
 * be valid.
 * @since v0.0.0.13
 *
 * @param[in, out] file The file structure to be operated on. The file's @c size
 * property is set by this function.
 *
 * @return A boolean value representing whether or not the file's size was
 * polled successfully. On failure, a message will be posted to @c stderr
 * alongside the current @c ERRNO value. This function typically fails because
 * of IO errors.
 */
[[gnu::nonnull(1)]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_getFileSize(waterlily_file_t *file);

/**
 * @fn bool waterlily_executeFile(const waterlily_file_t *const file, const char
 * *const *const argv, size_t argc, int *status)
 * @brief Execute the given file as a child process. The given file must have a
 * valid basename and point to a file which the process has execution rights
 * over.
 * @since v0.0.0.14
 *
 * @param[in] file The file structure to be operated on.
 * @param[in] argv Command-line arguments to be provided to the executable.
 * @param[in] argc The count of arguments provided.
 * @param[out] status The return status of the child process, should it return
 * from execution properly.
 *
 * @return A boolean value representing whether or not the file was executed
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function typically fails because of child
 * process-related errors.
 */
[[gnu::nonnull(1, 2, 4)]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_executeFile(const waterlily_file_t *const file,
                           const char *const *const argv, size_t argc,
                           int *status);

/**
 * @fn bool waterlily_glslToSPIRV(const waterlily_file_t *const file)
 * @brief Compile a given GLSL shader file to a SPIRV output utilizing the @c
 * glslang executable. This function will throw errors unless the @c glslang
 * executable is available on the current system. The given GLSL shader file
 * must have a valid basename.
 * @since v0.0.0.14
 *
 * @param[in] file The file structure to be operated on.
 *
 * @return A boolean value representing whether or not the file was compiled
 * successfully. On failure, a message will be posted to @c stderr alongside the
 * current @c ERRNO value. This function typically fails because of GLSL
 * validation errors or missing files.
 */
[[gnu::nonnull(1)]] [[gnu::cold]] [[nodiscard("Expression result unchecked.")]]
bool waterlily_glslToSPIRV(const waterlily_file_t *const file);

/**
 * @fn void waterlily_createFilepath(const waterlily_file_t *const file, char
 * *path)
 * @brief A function to generate a full file path from the filetype and basename
 * of the given file. The given file must have a valid basename and type.
 * @since v0.0.0.14
 *
 * @param[in] file The file structure to operate on.
 * @param[out] path The generated path. This buffer should be at least as large
 * as specified in @ref waterlily_MAX_PATH_LENGTH.
 */
[[gnu::nonnull(1, 2)]] [[gnu::flatten]]
void waterlily_createFilepath(const waterlily_file_t *const file, char *path);

[[gnu::nonnull(1)]] [[gnu::hot]] [[nodiscard("Expression result unchecked.")]]
static inline bool waterlily_fileExists(const waterlily_file_t *const file)
{
    char path[waterlily_MAX_PATH_LENGTH];
    waterlily_createFilepath(file, path);
    return access(path, F_OK) == 0;
}

[[gnu::nonnull(1)]]
void waterlily_splitStem(const char *const original, char *filename,
                         char *extension);

bool waterlily_changeDirectory(const char *const path);

#define waterlily_CONCURRENT_FRAMES 2

bool waterlily_getExtensions(char **storage);

bool waterlily_create(const char *name, uint32_t version);
void waterlily_destroy(void);

bool waterlily_compileShaders(const char **names, size_t count);

bool waterlily_render(uint32_t framebufferWidth, uint32_t framebufferHeight);

bool waterlily_sync(void);
/**
 * @fn bool waterlily_create(void)
 * @brief Create the main window object of the engine. This should only be
 * called once, to prevent resource wasting and other undesirable behavior.
 * There are no checks in this function for anything but internal failure, call
 * only when you're certain there is no other window created.
 * @since v0.0.0.1
 *
 * @remark The created window is always fullscreen, undecorated, and focused off
 * the bat.
 *
 * @param[in] title The title you wish your window to have. This must be
 * NUL-terminated, it is not edited in any way during the course of the
 * function.
 * @return A boolean value representing whether or not the window was created
 * successfully. A message will always be logged to an attached @c tty
 * explaining any errors.
 */
[[nodiscard]] [[gnu::nonnull(1)]]
bool waterlily_createWindow(const char *const title);

/**
 * @fn void waterlily_destroy(void)
 * @brief Destroy the main window object of the engine. This should only be
 * called when a window is truly created, to prevent double-frees and other
 * undefined behaviors. There are no checks in this function for anything
 * but internal failure, call only when you're certain there is a window
 * created.
 * @since v0.0.0.1
 *
 * @remark This function kills the window without asking for permission from
 * any other running processes. All other dependents (like graphical
 * processes) should be deinitialized before this function is called. To
 * close the window, please see @ref waterlily_close.
 */
void waterlily_destroyWindow(void);

/**
 * @fn bool waterlily_process(void)
 * @brief Process any and all window events and clear the queue. This should be
 * called each time you wish to paint a "frame" for the window. This will
 * process things like user input, close events, etc.
 * @since v0.0.0.2
 *
 * @return A boolean value representing whether or not event processing
 * succeeded. If false is returned, the window should close, no questions asked.
 * The window processing failing does not necessarily mean an error has
 * occurred, simply that processing cannot continue.
 */
[[nodiscard]] [[gnu::hot]]
bool waterlily_processWindow(void);

/**
 * @fn void waterlily_close(void)
 * @brief Close the window. This sends a bullet directly into the windowing
 * processes, and should alert its dependents to clean themselves up as well.
 * @since v0.0.0.2
 *
 * @remark This does not destroy the window. For that you should see @ref
 * waterlily_destroy, which should be called after this function.
 */
void waterlily_closeWindow(void);

/**
 * @fn void waterlily_getSize(uint32_t *width, uint32_t *height)
 * @brief Get the size of the window's framebuffer in pixels.
 * @since v0.0.0.9
 *
 * @remark On most platforms, this will equal the size in screen coordinates,
 * but on some displays like the Apple Retina, this is not the case.
 *
 * @param[out] width The storage for the width of the framebuffer in pixels.
 * @param[out] height The storage for the height of the framebuffer in pixels.
 */
[[gnu::nonnull(1, 2)]]
void waterlily_getSizeWindow(uint32_t *width, uint32_t *height);

/**
 * @fn void waterlily_getData(void **data)
 * @brief Get the native data specific to this window. Each platform has its own
 * set of data.
 *
 * @remark the following data is provided for each platform, in the order
 * specified; Wayland: @c wl_display, @c wl_surface, X11: N/A.
 *
 * @param[out] data The data array. This is not touched by the function, so it
 * must contain enough space to properly handle all items passed to it.
 */
[[gnu::nonnull(1)]]
void waterlily_getDataWindow(void **data);

#endif // WATERLILY_MAIN_H

