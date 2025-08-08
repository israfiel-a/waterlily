#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#define WATERLILY_CONCURRENT_FRAMES 2
#define WATERLILY_KEY_TIMER_MS 50

typedef struct waterlily_vulkan_queue
{
    uint32_t index;
    VkQueue handle;
} waterlily_vulkan_queue_t;

typedef enum waterlily_key_state
{
    WATERLILY_KEY_STATE_DOWN,
    WATERLILY_KEY_STATE_UP,
    WATERLILY_KEY_STATE_REPEAT
} waterlily_key_state_t;

typedef struct waterlily_key
{
    uint64_t timestamp;
    xkb_keysym_t symbol;
    xkb_keycode_t scancode;
    waterlily_key_state_t state;
} waterlily_key_t;

typedef struct waterlily_context
{
    VkInstance vulkan;
    uint32_t currentFrame;
    struct
    {
        VkPhysicalDevice physical;
        VkDevice logical;
    } gpu;
    struct
    {
        struct
        {
            bool displayFPS : 1;
        } flags;
        char *requiredDirectory;
        size_t requiredDirectoryLength;
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
        VkCommandPool pool;
        VkCommandBuffer buffers[WATERLILY_CONCURRENT_FRAMES];
    } commandBuffers;
} waterlily_context_t;

typedef struct waterlily_key_combination
{
    waterlily_key_t first;
    waterlily_key_t second;
    void (*func)(waterlily_key_t *first, waterlily_key_t *second,
                 waterlily_context_t *context);
} waterlily_key_combination_t;

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

typedef enum waterlily_close_type : uint8_t
{
    WATERLILY_CLOSE_GET,
    WATERLILY_CLOSE_ON,
    WATERLILY_CLOSE_OFF
} waterlily_close_type_t;

typedef enum waterlily_resize_type : uint8_t
{
    WATERLILY_RESIZE_GET,
    WATERLILY_RESIZE_YES,
    WATERLILY_RESIZE_NO
} waterlily_resize_type_t;

typedef struct waterlily_vulkan_pipeline_info
{
    VkPipelineVertexInputStateCreateInfo input;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlend;
    VkDynamicState dynamicState[2];
    VkPipelineDynamicStateCreateInfo dynamic;
} waterlily_vulkan_pipeline_info_t;

static const char *const waterlily_vulkan_gExtensions[] = {
    "VK_KHR_surface",
    "VK_KHR_wayland_surface",
};

static const char *const waterlily_vulkan_gDeviceExtensions[] = {
    "VK_KHR_swapchain",
};

#define waterlily_engine_log(type, format, ...)                                \
    waterlily_engine_log(                                                      \
        &(waterlily_log_t){WATERLILY_LOG_TYPE_##type, __LINE__, FILENAME},     \
        format __VA_OPT__(, ) __VA_ARGS__)

bool waterlily_engine_digest(int argc, const char *const *const argv,
                             waterlily_context_t *context);
bool waterlily_engine_setup(waterlily_context_t *context);
void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...);

bool waterlily_window_create(const char *const title,
                             waterlily_context_t *context);
void waterlily_window_destroy(waterlily_context_t *context);
bool waterlily_window_process(waterlily_context_t *context);

bool waterlily_vulkan_create(waterlily_context_t *context);
void waterlily_vulkan_destroy(waterlily_context_t *context);
bool waterlily_vulkan_render(waterlily_context_t *context);
bool waterlily_vulkan_sync(waterlily_context_t *context);

bool waterlily_vulkan_getPhysicalGPU(waterlily_context_t *context);
bool waterlily_vulkan_createLogicalGPU(waterlily_context_t *context);
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
    waterlily_context_t *context, const char *const *const stages, size_t count,
    VkPipelineShaderStageCreateInfo *storage);
void waterlily_vulkan_fillInfoPipeline(waterlily_vulkan_pipeline_info_t *info);
bool waterlily_vulkan_createLayoutPipeline(waterlily_context_t *context);
bool waterlily_vulkan_createRenderpassPipeline(waterlily_context_t *context);
bool waterlily_vulkan_createPipeline(waterlily_context_t *context,
                                     VkPipelineShaderStageCreateInfo *stages,
                                     size_t stageCount,
                                     waterlily_vulkan_pipeline_info_t *info);
void waterlily_vulkan_destroyPipeline(waterlily_context_t *context);

bool waterlily_vulkan_createBuffersCommand(waterlily_context_t *context);
bool waterlily_vulkan_createSyncsCommand(waterlily_context_t *context);
void waterlily_vulkan_beginRenderpassCommand(waterlily_context_t *context);
bool waterlily_vulkan_recordBufferCommand(waterlily_context_t *context);
void waterlily_vulkan_destroyBuffers(waterlily_context_t *context);
void waterlily_vulkan_destroySyncs(waterlily_context_t *context);

bool waterlily_files_open(const char *const path, FILE **file);
static inline void waterlily_files_close(FILE *file) { fclose(file); }
bool waterlily_files_measure(FILE *file, size_t *length);
bool waterlily_files_read(FILE *file, size_t count, uint8_t *buffer);
bool waterlily_files_execute(char *const *args);

bool waterlily_input_createContext(waterlily_context_t *context);
bool waterlily_input_setKeymap(waterlily_context_t *context,
                               const char *const string);
bool waterlily_input_createState(waterlily_context_t *context);
void waterlily_input_updateModifiers(waterlily_context_t *context,
                                     uint32_t depressed, uint32_t latched,
                                     uint32_t locked, uint32_t group);
void waterlily_input_destroy(waterlily_context_t *context);
void waterlily_input_checkKeys(waterlily_context_t *context,
                               waterlily_key_combination_t *keys, size_t count);

#endif // WATERLILY_MAIN_H

