#include <Waterlily.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

static uint32_t currentFrame = 0;

static VkSemaphore pImageAvailableSemaphores[waterlily_CONCURRENT_FRAMES];
static VkSemaphore pRenderFinishedSemaphores[waterlily_CONCURRENT_FRAMES];
static VkFence pFences[waterlily_CONCURRENT_FRAMES];

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
