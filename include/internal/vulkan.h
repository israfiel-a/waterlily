#ifndef WATERLILY_INTERNAL_VULKAN_H
#define WATERLILY_INTERNAL_VULKAN_H

#include "window.h"

#include <vulkan/vulkan.h>

#define WATERLILY_CONCURRENT_FRAMES 2

struct waterlily_vulkan_queue
{
    uint32_t index;
    VkQueue handle;
};

struct waterlily_vulkan_context
{
    VkInstance instance;
    uint32_t currentFrame;
#if BUILD_TYPE == 0
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    struct
    {
        VkPhysicalDevice physical;
        VkDevice logical;
        struct waterlily_vulkan_queue graphicsQueue;
        struct waterlily_vulkan_queue presentQueue;
    } gpu;
    struct
    {
        VkSurfaceKHR handle;
        VkPresentModeKHR mode;
        VkSurfaceFormatKHR format;
        VkExtent2D extent;
        VkSurfaceCapabilitiesKHR info;
    } surface;
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
};

struct waterlily_vulkan_context *
waterlily_createVulkanContext(struct waterlily_window_context *window);
void waterlily_destroyVulkanContext(void);
void waterlily_renderFrame(void);

#endif // WATERLILY_INTERNAL_VULKAN_H

