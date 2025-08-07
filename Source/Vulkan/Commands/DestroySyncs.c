#include <Waterlily.h>

void waterlily_vulkan_destroySyncs(VkDevice device,
                                   VkSemaphore *imageAvailableSemaphores,
                                   VkSemaphore *renderFinishedSemaphores,
                                   VkFence *fences)
{
    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, fences[i], nullptr);
    }
}

