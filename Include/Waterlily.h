#ifndef WATERLILY_MAIN_H
#define WATERLILY_MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

typedef struct waterlily_arguments
{
    struct
    {
        bool displayFPS : 1;
    } flags;
    char *requiredDirectory;
    size_t requiredDirectoryLength;
} waterlily_arguments_t;

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

typedef struct waterlily_vulkan_queue_indices
{
    uint32_t graphics;
    uint32_t present;
} waterlily_vulkan_queue_indices_t;

typedef struct waterlily_vulkan_queues
{
    VkQueue graphics;
    VkQueue present;
} waterlily_vulkan_queues_t;

typedef struct waterlily_vulkan_surface
{
    VkSurfaceKHR surface;
    VkPresentModeKHR mode;
    VkSurfaceFormatKHR format;
    VkSurfaceCapabilitiesKHR capabilities;
    VkExtent2D extent;
} waterlily_vulkan_surface_t;

typedef struct waterlily_vulkan_pipeline_info
{
    VkPipelineVertexInputStateCreateInfo input;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkViewport viewportData;
    VkRect2D scissorData;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlend;
} waterlily_vulkan_pipeline_info_t;

typedef struct waterlily_vulkan_graphics_pipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkRenderPass renderpass;
} waterlily_vulkan_graphics_pipeline_t;

extern const char *const waterlily_vulkan_gExtensions[2];
static const char *const waterlily_vulkan_gDeviceExtensions[] = {
    "VK_KHR_swapchain",
};

#define waterlily_engine_log(type, format, ...)                                \
    waterlily_engine_log(                                                      \
        &(waterlily_log_t){WATERLILY_LOG_TYPE_##type, __LINE__, FILENAME},     \
        format __VA_OPT__(, ) __VA_ARGS__)

bool waterlily_engine_digest(int argc, const char *const *const argv,
                             waterlily_arguments_t *arguments);
bool waterlily_engine_setup(const waterlily_arguments_t *const arguments);
void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...);

bool waterlily_window_create(const char *const title);
void waterlily_window_destroy(void);
void waterlily_window_measure(uint32_t *width, uint32_t *height);
void waterlily_window_getData(void **data);
bool waterlily_window_process(void);
static inline bool waterlily_window_close(waterlily_close_type_t type)
{
    static bool close = false;
    if (type == WATERLILY_CLOSE_ON)
        close = true;
    else if (type == WATERLILY_CLOSE_OFF)
        close = false;

    return close;
}

bool waterlily_vulkan_create(VkInstance *instance);
void waterlily_vulkan_destroy(VkInstance instance);

bool waterlily_vulkan_getPhysicalGPU(VkInstance instance,
                                     VkPhysicalDevice *device,
                                     waterlily_vulkan_queue_indices_t *indices,
                                     const VkSurfaceKHR surface);
bool waterlily_vulkan_createLogicalGPU(
    const VkPhysicalDevice physical, VkDevice *device,
    waterlily_vulkan_queues_t *queues,
    const waterlily_vulkan_queue_indices_t *indices);
void waterlily_vulkan_destroyGPU(VkDevice logical);

bool waterlily_vulkan_createSurface(VkInstance instance,
                                    waterlily_vulkan_surface_t *surface);
bool waterlily_vulkan_getFormatSurface(VkPhysicalDevice device,
                                       waterlily_vulkan_surface_t *surface);
bool waterlily_vulkan_getCapabilitiesSurface(
    VkPhysicalDevice device, waterlily_vulkan_surface_t *surface);
void waterlily_vulkan_getExtentSurface(uint32_t width, uint32_t height,
                                       waterlily_vulkan_surface_t *surface);
bool waterlily_vulkan_getModeSurface(VkPhysicalDevice device,
                                     waterlily_vulkan_surface_t *surface);

bool waterlily_vulkan_createSwapchain(
    VkDevice device, uint32_t *imageCount, VkSwapchainKHR *swapchain,
    waterlily_vulkan_surface_t *surface,
    waterlily_vulkan_queue_indices_t *indices);
bool waterlily_vulkan_partitionSwapchain(VkDevice device,
                                         waterlily_vulkan_surface_t *surface,
                                         VkSwapchainKHR swapchain,
                                         uint32_t imageCount,
                                         VkImageView *images);

bool waterlily_vulkan_setupShadersPipeline(
    const char *const *const stages, size_t count,
    VkPipelineShaderStageCreateInfo *storage);
void waterlily_vulkan_fillInfoPipeline(waterlily_vulkan_surface_t *surface,
                                       waterlily_vulkan_pipeline_info_t *info);
bool waterlily_vulkan_createLayoutPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline);
bool waterlily_vulkan_createRenderpassPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline,
    waterlily_vulkan_surface_t *surface);
bool waterlily_vulkan_createPipeline(
    VkDevice device, waterlily_vulkan_graphics_pipeline_t *pipeline,
    VkPipelineShaderStageCreateInfo *stages, size_t stageCount,
    waterlily_vulkan_pipeline_info_t *info);

bool waterlily_files_open(const char *const path, FILE *file);
static inline void waterlily_files_close(FILE *file) { fclose(file); }
bool waterlily_files_measure(FILE *file, size_t *length);
bool waterlily_files_read(FILE *file, size_t count, uint8_t *buffer);

#endif // WATERLILY_MAIN_H

