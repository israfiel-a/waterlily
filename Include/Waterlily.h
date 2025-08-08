#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#define WATERLILY_CONCURRENT_FRAMES 2

typedef struct waterlily_vulkan_queue
{
    uint32_t index;
    VkQueue handle;
} waterlily_vulkan_queue_t;

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
        union
        {
            struct
            {
                void *display;
                int screen;
                int root;
                unsigned long window;
            } x11;
            struct
            {
                void *display;
                void *registry;
                void *compositor;
                void *output;
                void *surface;
                void *shell;
                void *shellSurface;
                void *toplevel;
            } wayland;
        } data;
    } window;
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

extern const char *const waterlily_vulkan_gExtensions[2];
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

#endif // WATERLILY_MAIN_H

