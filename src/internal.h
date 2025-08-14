#ifndef WATERLILY_INTERNAL_H
#define WATERLILY_INTERNAL_H

#include <waterlily.h>
#include <vulkan/vulkan.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

typedef struct waterlily_vulkan_queue
{
    uint32_t index;
    VkQueue handle;
} waterlily_vulkan_queue_t;

typedef struct waterlily_context
{
    VkInstance vulkan;
#if BUILD_TYPE == 0
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    uint32_t currentFrame;
    struct
    {
        VkPhysicalDevice physical;
        VkDevice logical;
    } gpu;
    struct
    {
        bool displayFPS : 1;
    } arguments;
    struct
    {
        waterlily_vulkan_queue_t graphics;
        waterlily_vulkan_queue_t present;
    } queues;
    struct
    {
        VkSurfaceKHR surface;
        VkPresentModeKHR mode;
        VkSurfaceFormatKHR format;
        VkSurfaceCapabilitiesKHR capabilities;
        VkExtent2D extent;
        uint32_t scale;
        void *handle;
        bool close;
        bool resized;
        struct
        {
            struct wl_display *display;
            struct wl_registry *registry;
            struct wl_compositor *compositor;
            struct wl_output *output;
            struct wl_seat *seat;
            struct wl_keyboard *keyboard;
            struct wl_surface *surface;
            void *shell;
            void *shellSurface;
            void *toplevel;
        } data;
    } window;
    struct
    {
        struct xkb_context *context;
        struct xkb_state *state;
        waterlily_key_t down[2];
        waterlily_key_combination_t *combinations;
        size_t combinationCount;
    } input;
    struct
    {
        VkPipeline handle;
        VkPipelineLayout layout;
        VkRenderPass renderpass;
    } pipeline;
    struct
    {
        VkSwapchainKHR handle;
        VkImageView *images;
        VkFramebuffer *framebuffers;
        uint32_t imageCount;
    } swapchain;
    struct
    {
        VkSemaphore imageAvailableSemphores[WATERLILY_CONCURRENT_FRAMES];
        VkSemaphore renderFinishedSemaphores[WATERLILY_CONCURRENT_FRAMES];
        VkFence fences[WATERLILY_CONCURRENT_FRAMES];
        VkFence presentFence;
        VkCommandPool pool;
        VkCommandBuffer buffers[WATERLILY_CONCURRENT_FRAMES];
    } commandBuffers;
} waterlily_context_t;

typedef enum waterlily_log_type : uint8_t
{
    WATERLILY_LOG_TYPE_INFO,
    WATERLILY_LOG_TYPE_SUCCESS,
    WATERLILY_LOG_TYPE_WARNING,
    WATERLILY_LOG_TYPE_ERROR
} waterlily_log_type_t;

typedef struct waterlily_log
{
    const waterlily_log_type_t type;
    const size_t line;
    const char *const filename;
} waterlily_log_t;

typedef struct waterlily_configuration
{
    char *title;
} waterlily_configuration_t;

#define waterlily_engine_log(type, format, ...)                                \
    waterlily_engine_log(                                                      \
        &(waterlily_log_t){WATERLILY_LOG_TYPE_##type, __LINE__, FILENAME},     \
        format __VA_OPT__(, ) __VA_ARGS__)

bool waterlily_engine_digest(waterlily_context_t *context, int argc,
                             const char *const *const argv,
                             char **workingDirectory,
                             size_t *workingDirectoryLength);
void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...);
bool waterlily_engine_configure(waterlily_configuration_t *configuration);

bool waterlily_window_create(const char *const title,
                             waterlily_context_t *context);
void waterlily_window_destroy(waterlily_context_t *context);
bool waterlily_window_process(waterlily_context_t *context);

bool waterlily_vulkan_create(waterlily_context_t *context,
                             const char *const *const extensions, size_t count);
void waterlily_vulkan_destroy(waterlily_context_t *context);
bool waterlily_vulkan_render(waterlily_context_t *context);
bool waterlily_vulkan_sync(waterlily_context_t *context);

bool waterlily_vulkan_getPhysicalGPU(waterlily_context_t *context,
                                     const char *const *const extensions,
                                     size_t count);
bool waterlily_vulkan_createLogicalGPU(waterlily_context_t *context,
                                       const char *const *const extensions,
                                       size_t count,
                                       VkPhysicalDeviceFeatures2 *features);
void waterlily_vulkan_destroyGPU(waterlily_context_t *context);

bool waterlily_vulkan_createSurface(waterlily_context_t *context);
bool waterlily_vulkan_getFormatSurface(waterlily_context_t *context);
bool waterlily_vulkan_getCapabilitiesSurface(waterlily_context_t *context);
void waterlily_vulkan_getExtentSurface(waterlily_context_t *context);
bool waterlily_vulkan_getModeSurface(waterlily_context_t *context);
void waterlily_vulkan_destroySurface(waterlily_context_t *context);

bool waterlily_vulkan_createSwapchain(waterlily_context_t *context);
bool waterlily_vulkan_partitionSwapchain(waterlily_context_t *context);
bool waterlily_vulkan_createFramebuffersSwapchain(waterlily_context_t *context);
void waterlily_vulkan_destroySwapchain(waterlily_context_t *context);
bool waterlily_vulkan_recreateSwapchain(waterlily_context_t *context);

bool waterlily_vulkan_setupShadersPipeline(
    waterlily_context_t *context, VkPipelineShaderStageCreateInfo *storage);
bool waterlily_vulkan_createLayoutPipeline(waterlily_context_t *context);
bool waterlily_vulkan_createRenderpassPipeline(waterlily_context_t *context);
bool waterlily_vulkan_createPipeline(waterlily_context_t *context,
                                     VkPipelineShaderStageCreateInfo *stages,
                                     size_t stageCount);
void waterlily_vulkan_destroyPipeline(waterlily_context_t *context);

bool waterlily_vulkan_createBuffersCommand(waterlily_context_t *context);
bool waterlily_vulkan_createSyncsCommand(waterlily_context_t *context);
void waterlily_vulkan_beginRenderpassCommand(waterlily_context_t *context,
                                             uint32_t imageIndex);
bool waterlily_vulkan_recordBufferCommand(waterlily_context_t *context,
                                          uint32_t imageIndex);
void waterlily_vulkan_destroyBuffers(waterlily_context_t *context);
void waterlily_vulkan_destroySyncs(waterlily_context_t *context);

bool waterlily_files_open(const char *const path, FILE **file);
static inline void waterlily_files_close(FILE *file) { fclose(file); }
bool waterlily_files_measure(FILE *file, size_t *length);
bool waterlily_files_read(FILE *file, size_t count, uint8_t *buffer);
bool waterlily_files_execute(char *const *args);
bool waterlily_files_remove(const char *const path);
bool waterlily_files_exists(const char *const path);

bool waterlily_input_createContext(waterlily_context_t *context);
bool waterlily_input_setKeymap(waterlily_context_t *context,
                               const char *const string);
bool waterlily_input_createState(waterlily_context_t *context);
void waterlily_input_updateModifiers(waterlily_context_t *context,
                                     uint32_t depressed, uint32_t latched,
                                     uint32_t locked, uint32_t group);
void waterlily_input_destroy(waterlily_context_t *context);
void waterlily_input_checkKeys(waterlily_context_t *context);

#endif // WATERLILY_INTERNAL_H

