#ifndef WATERLILY_RAW_H
#define WATERLILY_RAW_H

#include <Waterlily.h>

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

#define waterlily_engine_log(type, format, ...)                                \
    waterlily_engine_log(                                                      \
        &(waterlily_log_t){WATERLILY_LOG_TYPE_##type, __LINE__, FILENAME},     \
        format __VA_OPT__(, ) __VA_ARGS__)

bool waterlily_engine_digest(waterlily_context_t *context, int argc,
                             const char *const *const argv);
void(waterlily_engine_log)(const waterlily_log_t *data,
                           const char *const format, ...);

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
                                       size_t count);
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

#endif // WATERLILY_RAW_H

