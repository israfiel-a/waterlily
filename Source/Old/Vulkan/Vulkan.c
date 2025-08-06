#include <Waterlily.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

extern bool createPipeline(const VkExtent2D *const extent,
                           const VkDevice device, VkFormat format);
extern void beginRenderpass(VkFramebuffer framebuffer, VkCommandBuffer buffer,
                            const VkExtent2D *const extent);

extern VkPresentModeKHR getSurfaceMode([[maybe_unused]] VkPhysicalDevice device);
extern VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice device);
extern VkExtent2D getSurfaceExtent(uint32_t width, uint32_t height);
extern VkSurfaceFormatKHR getSurfaceFormat([[maybe_unused]] VkPhysicalDevice device);

extern VkRenderPass gRenderpass;
extern VkSurfaceKHR gSurface;

static uint32_t currentFrame = 0;

static VkInstance pInstance = nullptr;
static VkPhysicalDevice pPhysicalDevice = nullptr;
static VkDevice pLogicalDevice = nullptr;
static VkQueue pGraphicsQueue = nullptr;
static VkQueue pPresentQueue = nullptr;
static uint32_t pGraphicsIndex = 0;
static uint32_t pPresentIndex = 0;

static VkSwapchainKHR pSwapchain = nullptr;
static uint32_t pImageCount = 0;
static VkImageView *pSwapchainImages = nullptr;
static VkFramebuffer *pSwapchainFramebuffers = nullptr;

static VkCommandPool pCommandPool;
static VkCommandBuffer pCommandBuffers[waterlily_CONCURRENT_FRAMES];

static VkSemaphore pImageAvailableSemaphores[waterlily_CONCURRENT_FRAMES];
static VkSemaphore pRenderFinishedSemaphores[waterlily_CONCURRENT_FRAMES];
static VkFence pFences[waterlily_CONCURRENT_FRAMES];

bool createFramebuffers(const VkExtent2D *const extent)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = gRenderpass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = extent->width;
    framebufferInfo.height = extent->height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < pImageCount; i++)
    {
        framebufferInfo.pAttachments = &pSwapchainImages[i];
        if (vkCreateFramebuffer(pLogicalDevice, &framebufferInfo, nullptr,
                                &pSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create framebuffer.\n");
            return false;
        }
    }
    return true;
}

bool createCommandBuffers(void)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = pGraphicsIndex;

    if (vkCreateCommandPool(pLogicalDevice, &poolInfo, nullptr,
                            &pCommandPool) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command pool.\n");
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = waterlily_CONCURRENT_FRAMES;

    if (vkAllocateCommandBuffers(pLogicalDevice, &allocInfo, pCommandBuffers) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command buffer.\n");
        return false;
    }

    return true;
}

bool createSyncObjects(void)
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < waterlily_CONCURRENT_FRAMES; i++)
    {
        if (vkCreateSemaphore(pLogicalDevice, &semaphoreInfo, nullptr,
                              &pImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(pLogicalDevice, &semaphoreInfo, nullptr,
                              &pRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(pLogicalDevice, &fenceInfo, nullptr, &pFences[i]) !=
                VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create sync object.\n");
            return false;
        }
    }
    return true;
}

bool recordCommandBuffer(VkCommandBuffer commandBuffer,
                         const VkExtent2D *extent, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to begin command buffer.\n");
        return false;
    }

    beginRenderpass(pSwapchainFramebuffers[imageIndex],
                    pCommandBuffers[currentFrame], extent);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to end command buffer.\n");
        return false;
    }

    return true;
}

bool createDevice(uint32_t framebufferWidth, uint32_t framebufferHeight)
{
    uint32_t physicalCount = 0;
    vkEnumeratePhysicalDevices(pInstance, &physicalCount, nullptr);
    VkPhysicalDevice *physicalDevices =
        malloc(sizeof(VkPhysicalDevice) * physicalCount);
    vkEnumeratePhysicalDevices(pInstance, &physicalCount, physicalDevices);

    const size_t extensionCount = 1;
    const char *extensions[1] = {"VK_KHR_swapchain"};

    VkPhysicalDevice currentChosen = nullptr;
    uint32_t bestScore = 0;
    for (size_t i = 0; i < physicalCount; i++)
    {
        uint32_t score =
            scoreDevice(physicalDevices[i], extensions, extensionCount);
        if (score > bestScore)
        {
            currentChosen = physicalDevices[i];
            bestScore = score;
        }
    }

    if (currentChosen == nullptr)
    {
        fprintf(stderr, "Failed to find suitable Vulkan device.\n");
        return false;
    }
    pPhysicalDevice = currentChosen;
    free(physicalDevices);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount,
                                             nullptr);

    VkQueueFamilyProperties *queueFamilies =
        malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount,
                                             queueFamilies);

    bool foundGraphicsQueue = false, foundPresentQueue = false;
    for (size_t i = 0; i < queueFamilyCount; i++)
    {
        if (foundGraphicsQueue && foundPresentQueue) break;

        VkQueueFamilyProperties family = queueFamilies[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            pGraphicsIndex = i;
            foundGraphicsQueue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevice, i, gSurface,
                                             &presentSupport);
        if (presentSupport)
        {
            pPresentIndex = i;
            foundPresentQueue = true;
        }
    }

    if (!foundGraphicsQueue)
    {
        fprintf(stderr, "Failed to find graphics queue.\n");
        return false;
    }

    if (!foundPresentQueue)
    {
        fprintf(stderr, "Failed to find presentation queue.\n");
        return false;
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {{0}, {0}};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = pGraphicsIndex;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &priority;

    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = pPresentIndex;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &priority;

    // We don't need any features at the moment.
    VkPhysicalDeviceFeatures usedFeatures = {0};

    // Layers for logical devices no longer need to be set in newer
    // implementations.
    VkDeviceCreateInfo logicalDeviceCreateInfo = {0};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    if (pPresentIndex != pGraphicsIndex)
        logicalDeviceCreateInfo.queueCreateInfoCount = 2;
    else logicalDeviceCreateInfo.queueCreateInfoCount = 1;
    logicalDeviceCreateInfo.pEnabledFeatures = &usedFeatures;

    logicalDeviceCreateInfo.enabledExtensionCount = extensionCount;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = extensions;

    VkResult code = vkCreateDevice(pPhysicalDevice, &logicalDeviceCreateInfo,
                                   nullptr, &pLogicalDevice);
    if (code != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create logical device. Code: %d.\n", code);
        return false;
    }

    vkGetDeviceQueue(pLogicalDevice, pGraphicsIndex, 0, &pGraphicsQueue);
    vkGetDeviceQueue(pLogicalDevice, pPresentIndex, 0, &pPresentQueue);

    getSurfaceCapabilities(pPhysicalDevice);
    VkExtent2D extent = getSurfaceExtent(framebufferWidth, framebufferHeight);
    if (!createSwapchain(&extent)) return false;
    if (!createPipeline(&extent, pLogicalDevice, getSurfaceFormat(pPhysicalDevice).format)) return false;
    if (!createFramebuffers(&extent)) return false;
    if (!createCommandBuffers()) return false;
    if (!createSyncObjects()) return false;

    return true;
}

void beginRenderpass(VkFramebuffer framebuffer, VkCommandBuffer buffer,
                     const VkExtent2D *const extent)
{
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = gRenderpass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = *extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pGraphicsPipeline);
}


bool waterlily_create(const char *name, uint32_t version)
{
    VkApplicationInfo applicationInfo = {0};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = name;
    applicationInfo.applicationVersion = version;
    applicationInfo.pEngineName = nullptr;
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;

    char *extensions[2];
    waterlily_getExtensions(extensions);
    instanceInfo.enabledExtensionCount = 2;
    instanceInfo.ppEnabledExtensionNames = (const char **)extensions;

    const char *layers[1] = {"VK_LAYER_KHRONOS_validation"};
    instanceInfo.enabledLayerCount = 1;
    instanceInfo.ppEnabledLayerNames = layers;

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &pInstance);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan instance. Code: %d.\n",
                result);
        return false;
    }

    // This is provided by the current target file.
    extern VkSurfaceKHR createSurface(VkInstance instance, void **data);
    void *data[2];
    waterlily_getDataWindow(data);
    gSurface = createSurface(pInstance, data);
    if (gSurface == nullptr) return false;

    uint32_t width, height;
    waterlily_getSizeWindow(&width, &height);
    if (!createDevice(width, height)) return false;

    return true;
}

void waterlily_destroy(void) { vkDeviceWaitIdle(pLogicalDevice); }

void cleanupSwapchain(void)
{
    for (size_t i = 0; i < pImageCount; i++)
    {
        vkDestroyFramebuffer(pLogicalDevice, pSwapchainFramebuffers[i],
                             nullptr);
        vkDestroyImageView(pLogicalDevice, pSwapchainImages[i], nullptr);
    }
    vkDestroySwapchainKHR(pLogicalDevice, pSwapchain, nullptr);
}

bool recreateSwapchain(const VkExtent2D *const extent)
{
    vkDeviceWaitIdle(pLogicalDevice);

    cleanupSwapchain();
    if (!createSwapchain(extent)) return false;
    if (!createFramebuffers(extent)) return false;

    return true;
}

bool waterlily_render(uint32_t framebufferWidth,
                                 uint32_t framebufferHeight)
{
    vkWaitForFences(pLogicalDevice, 1, &pFences[currentFrame], VK_TRUE,
                    UINT64_MAX);

    VkExtent2D extent = getSurfaceExtent(framebufferWidth, framebufferHeight);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        pLogicalDevice, pSwapchain, UINT64_MAX,
        pImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (!recreateSwapchain(&extent)) return false;
        return true;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        fprintf(stderr, "Failed to acquire swapchain image.\n");
        return false;
    }

    vkResetFences(pLogicalDevice, 1, &pFences[currentFrame]);

    vkResetCommandBuffer(pCommandBuffers[currentFrame], 0);
    if (!recordCommandBuffer(pCommandBuffers[currentFrame], &extent, imageIndex))
        return false;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {pImageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &pCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {pRenderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(pGraphicsQueue, 1, &submitInfo, pFences[currentFrame]) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Failed to submit to the queue.\n");
        return false;
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {pSwapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(pPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        if (!recreateSwapchain(&extent)) return false;
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to present swapchain image.\n");
        return false;
    }

    currentFrame = (currentFrame + 1) % waterlily_CONCURRENT_FRAMES;
    return true;
}

bool waterlily_sync(void)
{
    return vkDeviceWaitIdle(pLogicalDevice) == VK_SUCCESS;
}
