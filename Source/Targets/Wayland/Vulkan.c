#include <Waterlily.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <string.h>

VkSurfaceKHR createSurface(VkInstance instance, void **data)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = data[0];
    createInfo.surface = data[1];

    VkSurfaceKHR createdSurface;
    VkResult code = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr,
                                              &createdSurface);
    if (code != VK_SUCCESS)
    {
        fprintf(stderr,
                "Failed to create Vulkan-Wayland interop surface. Code: %d.\n",
                code);
        return nullptr;
    }
    return createdSurface;
}

bool waterlily_getExtensions(char **storage)
{
    const char *const required[2] = {"VK_KHR_surface",
                                     "VK_KHR_wayland_surface"};
    const size_t requiredExtensions = sizeof(required) / sizeof(const char *);

    uint32_t extensionCount;
    VkResult result = vkEnumerateInstanceExtensionProperties(
        nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS)
    {
        waterlily_log(ERROR,
                     "Failed to enumerate instance extensions. Code: %d.",
                     result);
        return false;
    }

    VkExtensionProperties extensions[extensionCount];
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                    extensions);
    if (result != VK_SUCCESS)
    {
        waterlily_log(ERROR,
                     "Failed to enumerate instance extensions. Code: %d.",
                     result);
        return false;
    }

    uint32_t foundExtensions = 0;
    for (size_t i = 0; i < extensionCount; i++)
    {
        if (foundExtensions == requiredExtensions) break;

        auto extension = extensions[i];
        waterlily_log(INFO, "Found extension '%s'.", extension.extensionName);
        for (size_t j = 0; j < requiredExtensions; j++)
            if (strcmp(extension.extensionName, required[j]) == 0)
            {
                storage[foundExtensions] = (char *)required[j];
                foundExtensions++;
                waterlily_log(SUCCESS, "Found required extension '%s.'",
                             extension.extensionName);
                break;
            }
    }

    if (foundExtensions != requiredExtensions)
    {
        waterlily_log(ERROR, "Failed to find all required extensions.");
        return false;
    }
    waterlily_log(SUCCESS, "Found all required extensions.");
    return true;
}

