#ifndef WATERLILY_H
#define WATERLILY_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
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
        VkFence presentFence;
        VkCommandPool pool;
        VkCommandBuffer buffers[WATERLILY_CONCURRENT_FRAMES];
    } commandBuffers;
} waterlily_context_t;

typedef struct waterlily_vulkan_pipeline_info
{
    VkPipelineVertexInputStateCreateInfo input;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendStateCreateInfo colorBlend;
    VkPipelineDynamicStateCreateInfo dynamic;
} waterlily_vulkan_pipeline_info_t;

#define WATERLILY_VULKAN_PIPELINE_INFO_DEFAULT                                   \
    (waterlily_vulkan_pipeline_info_t)                                           \
    {                                                                            \
        .input =                                                                 \
            {                                                                    \
                .sType =                                                         \
                    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,   \
                .pNext = nullptr,                                                \
                .flags = 0,                                                      \
                .vertexBindingDescriptionCount = 0,                              \
                .pVertexBindingDescriptions = nullptr,                           \
                .vertexAttributeDescriptionCount = 0,                            \
                .pVertexAttributeDescriptions = nullptr,                         \
            },                                                                   \
        .assembly =                                                              \
            {                                                                    \
                .sType =                                                         \
                    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, \
                .pNext = nullptr,                                                \
                .flags = 0,                                                      \
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                 \
                .primitiveRestartEnable = false,                                 \
            },                                                                   \
        .rasterizer =                                                            \
            {                                                                    \
                .sType =                                                         \
                    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,  \
                .pNext = nullptr,                                                \
                .flags = 0,                                                      \
                .depthClampEnable = false,                                       \
                .rasterizerDiscardEnable = false,                                \
                .polygonMode = VK_POLYGON_MODE_FILL,                             \
                .cullMode = VK_CULL_MODE_BACK_BIT,                               \
                .frontFace = VK_FRONT_FACE_CLOCKWISE,                            \
                .depthBiasEnable = false,                                        \
                .depthBiasConstantFactor = 0.0f,                                 \
                .depthBiasClamp = 0.0f,                                          \
                .depthBiasSlopeFactor = 0.0f,                                    \
                .lineWidth = 1.0f,                                               \
            },                                                                   \
        .multisampling =                                                         \
            {                                                                    \
                .sType =                                                         \
                    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,    \
                .pNext = nullptr,                                                \
                .flags = 0,                                                      \
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,                   \
                .sampleShadingEnable = false,                                    \
                .minSampleShading = 0,                                           \
                .pSampleMask = nullptr,                                          \
                .alphaToCoverageEnable = false,                                  \
                .alphaToOneEnable = false,                                       \
            },                                                                   \
        .colorBlend =                                                            \
            {                                                                    \
                .sType =                                                         \
                    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,    \
                .pNext = nullptr,                                                \
                .flags = 0,                                                      \
                .logicOpEnable = false,                                          \
                .logicOp = VK_LOGIC_OP_CLEAR,                                    \
                .attachmentCount = 1,                                            \
                .pAttachments = &colorAttachment,                                \
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},                      \
            },                                                                   \
        .dynamic = {                                                             \
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,       \
            .pNext = nullptr,                                                    \
            .flags = 0,                                                          \
            .dynamicStateCount = dynamicStateCount,                              \
            .pDynamicStates = &dynamicState[0],                                  \
        }                                                                        \
    }

typedef struct waterlily_configuration
{
    int argc;
    const char *const *const argv;
    const char *const title;
    struct
    {
        const char *const *const instanceExtensions;
        const char *const *const deviceExtensions;
        size_t instanceExtensionCount;
        size_t deviceExtensionCount;
        waterlily_vulkan_pipeline_info_t pipelineInfo;
        VkPhysicalDeviceFeatures2 deviceFeatures;
    } vulkan;
} waterlily_configuration_t;

typedef struct waterlily_key_combination
{
    waterlily_key_t first;
    waterlily_key_t second;
    void (*func)(waterlily_key_t *first, waterlily_key_t *second,
                 waterlily_context_t *context);
} waterlily_key_combination_t;

bool waterlily_initialize(waterlily_context_t *context,
                          waterlily_configuration_t *configuration);
void waterlily_deinitialize(waterlily_context_t *context);
bool waterlily_run(waterlily_context_t *context,
                   waterlily_key_combination_t *combinations,
                   size_t combinationCount);

#endif // WATERLILY_H

