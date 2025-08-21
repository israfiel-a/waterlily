#include "internal.h"
#include "internal/files.h"

#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_wayland.h>

bool waterlily_vulkan_createBuffersCommand(waterlily_context_t *context)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = context->queues.graphics.index;

    VkResult result =
        vkCreateCommandPool(context->gpu.logical, &poolInfo, nullptr,
                            &context->commandBuffers.pool);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command pool, code %d.",
                             result);
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context->commandBuffers.pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = WATERLILY_CONCURRENT_FRAMES;

    result = vkAllocateCommandBuffers(context->gpu.logical, &allocInfo,
                                      context->commandBuffers.buffers);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

bool waterlily_vulkan_createSyncsCommand(waterlily_context_t *context)
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateFence(context->gpu.logical, &fenceInfo, nullptr,
                                    &context->commandBuffers.presentFence);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create present fence, code %d.",
                             result);
        return false;
    }

    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        result = vkCreateSemaphore(
            context->gpu.logical, &semaphoreInfo, nullptr,
            &context->commandBuffers.imageAvailableSemphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create image available semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateSemaphore(
            context->gpu.logical, &semaphoreInfo, nullptr,
            &context->commandBuffers.renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR,
                "Failed to create render finished semaphore %zu, code %d.", i,
                result);
            return false;
        }

        result = vkCreateFence(context->gpu.logical, &fenceInfo, nullptr,
                               &context->commandBuffers.fences[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(ERROR, "Failed to create fence %zu, code %d.",
                                 i, result);
            return false;
        }
    }
    return true;
}

bool waterlily_vulkan_createLogicalGPU(waterlily_context_t *context,
                                       const char *const *const extensions,
                                       size_t count,
                                       VkPhysicalDeviceFeatures2 *features)
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {{0}, {0}};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = context->queues.graphics.index;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &priority;

    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = context->queues.present.index;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &priority;

    // Layers for logical devices no longer need to be set in newer
    // implementations.
    VkDeviceCreateInfo logicalDeviceCreateInfo = {0};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    logicalDeviceCreateInfo.queueCreateInfoCount =
        context->queues.graphics.index == context->queues.present.index ? 1 : 2;
    logicalDeviceCreateInfo.pEnabledFeatures = nullptr;
    logicalDeviceCreateInfo.pNext = features;

    logicalDeviceCreateInfo.enabledExtensionCount = count;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = extensions;

    VkResult code =
        vkCreateDevice(context->gpu.physical, &logicalDeviceCreateInfo, nullptr,
                       &context->gpu.logical);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create logical device. Code: %d.", code);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created logical device.");

    vkGetDeviceQueue(context->gpu.logical, context->queues.graphics.index, 0,
                     &context->queues.graphics.handle);
    vkGetDeviceQueue(context->gpu.logical, context->queues.present.index, 0,
                     &context->queues.present.handle);
    waterlily_engine_log(INFO, "Created device data queues.");
    return true;
}

static uint32_t scoreDevice(waterlily_context_t *context,
                            const char *const *const extensions, size_t count)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(context->gpu.physical, &properties);
    vkGetPhysicalDeviceFeatures(context->gpu.physical, &features);

    uint32_t score = 0;
    switch (properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 4;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 3;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score += 2;
            break;
        default:
            score += 1;
            break;
    }

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(context->gpu.physical, nullptr,
                                         &extensionCount, nullptr);
    VkExtensionProperties foundExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(context->gpu.physical, nullptr,
                                         &extensionCount, foundExtensions);

    size_t requiredCount = count;
    size_t foundCount = 0;
    for (size_t i = 0; i < extensionCount; ++i)
    {
        if (foundCount == requiredCount)
            break;

        VkExtensionProperties extension = foundExtensions[i];
        waterlily_engine_log(INFO, "Found extension '%s'.",
                             extension.extensionName);

        for (size_t j = 0; j < requiredCount; ++j)
            if (strcmp(extension.extensionName, extensions[j]) == 0)
            {
                foundCount++;
                waterlily_engine_log(SUCCESS, "Found '%s' device extension.",
                                     extension.extensionName);
                break;
            }
    }

    if (foundCount != requiredCount)
    {
        waterlily_engine_log(ERROR,
                             "Failed to find all required device extensions.");
        return 0;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context->gpu.physical,
                                             &queueFamilyCount, nullptr);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(context->gpu.physical,
                                             &queueFamilyCount, queueFamilies);

    bool foundGraphicsQueue = false, foundPresentQueue = false;
    for (size_t i = 0; i < queueFamilyCount; i++)
    {
        if (foundGraphicsQueue && foundPresentQueue)
            break;

        VkQueueFamilyProperties family = queueFamilies[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            context->queues.graphics.index = i;
            foundGraphicsQueue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            context->gpu.physical, i, context->window.surface, &presentSupport);
        if (presentSupport)
        {
            context->queues.present.index = i;
            foundPresentQueue = true;
        }
    }

    if (!foundGraphicsQueue || !foundPresentQueue)
    {
        waterlily_engine_log(ERROR, "Failed to find required queue.");
        return 0;
    }
    return score;
}

bool waterlily_vulkan_getPhysicalGPU(waterlily_context_t *context,
                                     const char *const *const extensions,
                                     size_t count)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->vulkan, &deviceCount, nullptr);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(context->vulkan, &deviceCount, devices);

    uint32_t bestScore = 0;
    for (size_t i = 0; i < deviceCount; i++)
    {
        context->gpu.physical = devices[i];
        uint32_t score = scoreDevice(context, extensions, count);
        if (score > bestScore)
            bestScore = score;
    }

    if (bestScore == 0)
    {
        waterlily_engine_log(ERROR, "Failed to find suitable Vulkan device.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Found suitable Vulkan device.");
    return true;
}

bool waterlily_vulkan_createPipeline(waterlily_context_t *context,
                                     VkPipelineShaderStageCreateInfo *stages,
                                     size_t stageCount)
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = stageCount;
    pipelineInfo.pStages = stages;
    pipelineInfo.pViewportState = &(VkPipelineViewportStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    pipelineInfo.pVertexInputState =
        &(struct VkPipelineVertexInputStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        };
    pipelineInfo.pInputAssemblyState = &(
        struct VkPipelineInputAssemblyStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = false,
    };
    pipelineInfo.pRasterizationState =
        &(struct VkPipelineRasterizationStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = false,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f,
        };
    pipelineInfo.pMultisampleState =
        &(struct VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = false,
            .minSampleShading = 0,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = false,
            .alphaToOneEnable = false,
        };
    pipelineInfo.pColorBlendState =
        &(struct VkPipelineColorBlendStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = false,
            .logicOp = VK_LOGIC_OP_CLEAR,
            .attachmentCount = 1,
            .pAttachments =
                &(struct VkPipelineColorBlendAttachmentState){
                    .blendEnable = false,
                    .srcColorBlendFactor = 0,
                    .dstColorBlendFactor = 0,
                    .colorBlendOp = 0,
                    .srcAlphaBlendFactor = 0,
                    .dstAlphaBlendFactor = 0,
                    .alphaBlendOp = 0,
                    .colorWriteMask =
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                },
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        };
    pipelineInfo.pDynamicState = &(struct VkPipelineDynamicStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates =
            (VkDynamicState[2]){
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            },
    };
    pipelineInfo.layout = context->pipeline.layout;
    pipelineInfo.renderPass = context->pipeline.renderpass;

    VkResult result = vkCreateGraphicsPipelines(context->gpu.logical, nullptr,
                                                1, &pipelineInfo, nullptr,
                                                &context->pipeline.handle);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create graphics pipeline. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created graphics pipeline.");

    for (size_t i = 0; i < stageCount; ++i)
        vkDestroyShaderModule(context->gpu.logical, stages[i].module, nullptr);
    return true;
}

bool waterlily_vulkan_createLayoutPipeline(waterlily_context_t *context)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result =
        vkCreatePipelineLayout(context->gpu.logical, &pipelineLayoutInfo,
                               nullptr, &context->pipeline.layout);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create pipeline layout. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created pipeline layout.");
    return true;
}

bool waterlily_vulkan_createRenderpassPipeline(waterlily_context_t *context)
{
    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription description = {0};
    description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    description.colorAttachmentCount = 1;
    description.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = context->window.format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &description;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result =
        vkCreateRenderPass(context->gpu.logical, &renderPassInfo, nullptr,
                           &context->pipeline.renderpass);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create renderpass. Code: %d.",
                             result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created renderpass.");
    return true;
}

bool waterlily_vulkan_createSurface(waterlily_context_t *context)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = context->window.data.display;
    createInfo.surface = context->window.data.surface;

    VkResult result = vkCreateWaylandSurfaceKHR(
        context->vulkan, &createInfo, nullptr, &context->window.surface);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan-Wayland interop surface. Code: %d.",
            result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan-Wayland interop surface.");

    return true;
}

bool waterlily_vulkan_getCapabilitiesSurface(waterlily_context_t *context)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context->gpu.physical, context->window.surface,
        &context->window.capabilities);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to get surface capabilities, code %d.\n", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Got surface capabilities.");
    return true;
}

bool waterlily_vulkan_createSwapchain(waterlily_context_t *context)
{
    context->swapchain.imageCount =
        context->window.capabilities.minImageCount + 1;
    if (context->window.capabilities.maxImageCount > 0 &&
        context->swapchain.imageCount >
            context->window.capabilities.maxImageCount)
        context->swapchain.imageCount =
            context->window.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context->window.surface;
    createInfo.minImageCount = context->swapchain.imageCount;
    createInfo.imageFormat = context->window.format.format;
    createInfo.imageColorSpace = context->window.format.colorSpace;
    createInfo.imageExtent = context->window.extent;
    createInfo.presentMode = context->window.mode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = context->window.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    if (context->queues.graphics.index != context->queues.present.index)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t indices[2] = {context->queues.graphics.index,
                               context->queues.present.index};
        createInfo.pQueueFamilyIndices = indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult code = vkCreateSwapchainKHR(context->gpu.logical, &createInfo,
                                         nullptr, &context->swapchain.handle);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to create swapchain, code %d.",
                             code);
        return false;
    }
    return true;
}

bool waterlily_vulkan_createFramebuffersSwapchain(waterlily_context_t *context)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = context->pipeline.renderpass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = context->window.extent.width;
    framebufferInfo.height = context->window.extent.height;
    framebufferInfo.layers = 1;

    context->swapchain.framebuffers =
        malloc(sizeof(VkFramebuffer) * context->swapchain.imageCount);

    for (size_t i = 0; i < context->swapchain.imageCount; ++i)
    {
        framebufferInfo.pAttachments = &context->swapchain.images[i];
        VkResult result =
            vkCreateFramebuffer(context->gpu.logical, &framebufferInfo, nullptr,
                                &context->swapchain.framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create framebuffer %zu, code %d.", i, result);
            return false;
        }
        waterlily_engine_log(SUCCESS, "Created framebuffer %zu.", i);
    }
    return true;
}

bool waterlily_vulkan_partitionSwapchain(waterlily_context_t *context)
{
    VkImage rawImages[context->swapchain.imageCount];
    VkResult code =
        vkGetSwapchainImagesKHR(context->gpu.logical, context->swapchain.handle,
                                &context->swapchain.imageCount, rawImages);
    if (code != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to get swapchain images, code %d.",
                             code);
        return false;
    }

    VkImageViewCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageCreateInfo.format = context->window.format.format;
    imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCreateInfo.subresourceRange.levelCount = 1;
    imageCreateInfo.subresourceRange.layerCount = 1;

    context->swapchain.images =
        malloc(sizeof(VkImageView) * context->swapchain.imageCount);
    for (size_t i = 0; i < context->swapchain.imageCount; i++)
    {
        imageCreateInfo.image = rawImages[i];
        VkResult result =
            vkCreateImageView(context->gpu.logical, &imageCreateInfo, nullptr,
                              &context->swapchain.images[i]);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create image view %zu, code %d.", i, result);
            return false;
        }
        waterlily_engine_log(SUCCESS, "Created image view %zu.", i);
    }
    return true;
}

#if BUILD_TYPE == 0
static VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *data)
{
    waterlily_context_t *context = data;
    context->window.close = true;

    waterlily_engine_log(ERROR, "Vulkan error recieved: '%s'.",
                         pCallbackData->pMessage);

    return VK_FALSE;
}
#endif

bool waterlily_vulkan_create(waterlily_context_t *context)
{
    VkApplicationInfo applicationInfo = {0};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = nullptr;
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = nullptr;
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;

    const char *extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface",
        "VK_KHR_get_surface_capabilities2",
        "VK_EXT_surface_maintenance1",
        "VK_EXT_debug_utils",
    };

    instanceInfo.enabledExtensionCount = sizeof(extensions) / sizeof(char *);
    instanceInfo.ppEnabledExtensionNames = extensions;

    uint32_t foundExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &foundExtensionCount,
                                           nullptr);
    VkExtensionProperties foundExtensions[foundExtensionCount];
    vkEnumerateInstanceExtensionProperties(nullptr, &foundExtensionCount,
                                           foundExtensions);

    size_t foundProvidedExtensions = 0;
    for (size_t i = 0; i < foundExtensionCount; ++i)
    {
        VkExtensionProperties *properties = &foundExtensions[i];
        waterlily_engine_log(INFO, "Found instance extension '%s'.",
                             properties->extensionName);
        for (size_t j = 0; j < instanceInfo.enabledExtensionCount; ++j)
        {
            if (strcmp(properties->extensionName, extensions[j]) == 0)
            {
                waterlily_engine_log(SUCCESS,
                                     "Found requested instance extension '%s'.",
                                     properties->extensionName);
                foundProvidedExtensions++;
                break;
            }
        }
    }

    if (foundProvidedExtensions != instanceInfo.enabledExtensionCount)
    {
        waterlily_engine_log(
            ERROR,
            "Failed to find the required amount of instance extensions.");
        return false;
    }
    waterlily_engine_log(SUCCESS, "Found all required instance extensions.");

#if BUILD_TYPE == 0
    constexpr size_t layerCount = 1;
    const char *const layers[layerCount] = {
        "VK_LAYER_KHRONOS_validation",
    };
#else
    constexpr size_t layerCount = 0;
    const char *const *const layers = nullptr;
#endif

    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = layers;

    VkResult result =
        vkCreateInstance(&instanceInfo, nullptr, &context->vulkan);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan instance. Code: %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan instance.");

#if BUILD_TYPE == 0
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = context;

    auto debugCreator =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->vulkan, "vkCreateDebugUtilsMessengerEXT");
    result = debugCreator(context->vulkan, &createInfo, nullptr,
                          &context->debugMessenger);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to create Vulkan debug messenger, code %d.", result);
        return false;
    }
    waterlily_engine_log(SUCCESS, "Created Vulkan debug messenger.");
#endif

    return true;
}

void waterlily_vulkan_destroyBuffers(waterlily_context_t *context)
{
    vkDestroyCommandPool(context->gpu.logical, context->commandBuffers.pool,
                         nullptr);
}

void waterlily_vulkan_destroySyncs(waterlily_context_t *context)
{
    vkDestroyFence(context->gpu.logical, context->commandBuffers.presentFence,
                   nullptr);
    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        vkDestroySemaphore(context->gpu.logical,
                           context->commandBuffers.imageAvailableSemphores[i],
                           nullptr);
        vkDestroySemaphore(context->gpu.logical,
                           context->commandBuffers.renderFinishedSemaphores[i],
                           nullptr);
        vkDestroyFence(context->gpu.logical, context->commandBuffers.fences[i],
                       nullptr);
    }
}

void waterlily_vulkan_destroyGPU(waterlily_context_t *context)
{
    waterlily_vulkan_sync(context);
    vkDestroyDevice(context->gpu.logical, nullptr);
}

void waterlily_vulkan_destroyPipeline(waterlily_context_t *context)
{
    vkDestroyRenderPass(context->gpu.logical, context->pipeline.renderpass,
                        nullptr);
    vkDestroyPipelineLayout(context->gpu.logical, context->pipeline.layout,
                            nullptr);
    vkDestroyPipeline(context->gpu.logical, context->pipeline.handle, nullptr);
}

void waterlily_vulkan_destroySurface(waterlily_context_t *context)
{
    vkDestroySurfaceKHR(context->vulkan, context->window.surface, nullptr);
}

void waterlily_vulkan_destroySwapchain(waterlily_context_t *context)
{
    for (size_t i = 0; i < context->swapchain.imageCount; ++i)
    {
        vkDestroyFramebuffer(context->gpu.logical,
                             context->swapchain.framebuffers[i], nullptr);
        vkDestroyImageView(context->gpu.logical, context->swapchain.images[i],
                           nullptr);
    }
    vkDestroySwapchainKHR(context->gpu.logical, context->swapchain.handle,
                          nullptr);
    free(context->swapchain.framebuffers);
    free(context->swapchain.images);
}

void waterlily_vulkan_destroy(waterlily_context_t *context)
{
#if BUILD_TYPE == 0
    auto debugDestroy =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->vulkan, "vkDestroyDebugUtilsMessengerEXT");
    debugDestroy(context->vulkan, context->debugMessenger, nullptr);
#endif

    vkDestroyInstance(context->vulkan, nullptr);
}

bool waterlily_vulkan_recordBufferCommand(waterlily_context_t *context,
                                          uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(
        context->commandBuffers.buffers[context->currentFrame], &beginInfo);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to begin command buffer, code %d.",
                             result);
        return false;
    }

    VkViewport viewport = {0};
    viewport.width = (float)context->window.extent.width;
    viewport.height = (float)context->window.extent.height;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent.width = context->window.extent.width;
    scissor.extent.height = context->window.extent.height;

    vkCmdSetViewport(context->commandBuffers.buffers[context->currentFrame], 0,
                     1, &viewport);
    vkCmdSetScissor(context->commandBuffers.buffers[context->currentFrame], 0,
                    1, &scissor);

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context->pipeline.renderpass;
    renderPassInfo.framebuffer = context->swapchain.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = context->window.extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(context->commandBuffers.buffers[context->currentFrame],
                         &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(context->commandBuffers.buffers[context->currentFrame],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      context->pipeline.handle);
    vkCmdDraw(context->commandBuffers.buffers[context->currentFrame], 3, 1, 0,
              0);
    vkCmdEndRenderPass(context->commandBuffers.buffers[context->currentFrame]);

    result = vkEndCommandBuffer(
        context->commandBuffers.buffers[context->currentFrame]);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to end command buffer, code %d.",
                             result);
        return false;
    }

    return true;
}

bool waterlily_vulkan_setupShadersPipeline(
    waterlily_context_t *context, VkPipelineShaderStageCreateInfo *storage)
{
    waterlily_file_t shaderFile = {
        .name = "shaders",
        .type = WATERLILY_SHADER_FILE,
    };
    if (!waterlily_readFile(&shaderFile))
        return false;

    for (size_t i = 0; i < WATERLILY_SHADER_STAGES; ++i)
    {
        VkPipelineShaderStageCreateInfo *info = &storage[i];
        info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info->pName = "main";

        switch (i)
        {
            case 0:
                info->stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case 1:
                info->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                waterlily_engine_log(ERROR, "Unknown shader stage index %d.",
                                     i);
                return false;
                break;
        }

        VkShaderModuleCreateInfo moduleCreate = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = shaderFile.shader[i].size,
            .pCode = shaderFile.shader[i].code,
        };
        VkResult result = vkCreateShaderModule(
            context->gpu.logical, &moduleCreate, nullptr, &info->module);
        if (result != VK_SUCCESS)
        {
            waterlily_engine_log(
                ERROR, "Failed to create shader module, code %d.", result);
            return false;
        }
    }

    return true;
}

// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#16659263
static inline uint32_t clamp(uint32_t d, uint32_t min, uint32_t max)
{
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

void waterlily_vulkan_getExtentSurface(waterlily_context_t *context)
{
    if (context->window.capabilities.currentExtent.width != UINT32_MAX)
    {
        context->window.extent = context->window.capabilities.currentExtent;
        return;
    }

    context->window.extent.width =
        clamp(context->window.extent.width,
              context->window.capabilities.minImageExtent.width,
              context->window.capabilities.maxImageExtent.width);
    context->window.extent.height =
        clamp(context->window.extent.height,
              context->window.capabilities.minImageExtent.height,
              context->window.capabilities.maxImageExtent.height);
}

bool waterlily_vulkan_getFormatSurface(waterlily_context_t *context)
{
    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->gpu.physical, context->window.surface, &formatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerate surface formats (1), code %d.", result);
        return false;
    }

    VkSurfaceFormatKHR formats[formatCount];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->gpu.physical, context->window.surface, &formatCount, formats);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerate surface formats (2), code %d.", result);
        return false;
    }

    for (size_t i = 0; i < formatCount; ++i)
    {
        VkSurfaceFormatKHR currentFormat = formats[i];
        // This is the best combination. If not available, we'll just
        // select the first provided colorspace.
        if (currentFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            context->window.format = currentFormat;
            waterlily_engine_log(SUCCESS, "Found optimal surface format.");
            return true;
        }
    }

    waterlily_engine_log(
        ERROR, "Could not find the optimal colorspace. Falling back on %dx%d.",
        formats[0].format, formats[0].colorSpace);
    context->window.format = formats[0];
    return true;
}

bool waterlily_vulkan_getModeSurface(waterlily_context_t *context)
{
    uint32_t modeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->gpu.physical, context->window.surface, &modeCount, nullptr);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerator present modes (1), code %d.", result);
        return false;
    }

    VkPresentModeKHR modes[modeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        context->gpu.physical, context->window.surface, &modeCount, modes);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to enumerator present modes (2), code %d.", result);
        return false;
    }

    for (size_t i = 0; i < modeCount; ++i)
    {
        // This is the best mode. If not available, we'll just select
        // VK_PRESENT_MODE_FIFO_KHR.
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            context->window.mode = modes[i];
            waterlily_engine_log(SUCCESS, "Found optimal present mode.");
            return true;
        }
    }

    waterlily_engine_log(ERROR, "Failed to find optimal present mode.");
    context->window.mode = VK_PRESENT_MODE_FIFO_KHR;
    return true;
}

bool waterlily_vulkan_recreateSwapchain(waterlily_context_t *context)
{
    waterlily_vulkan_sync(context);
    waterlily_vulkan_destroySwapchain(context);

    if (!waterlily_vulkan_createSwapchain(context) ||
        !waterlily_vulkan_partitionSwapchain(context) ||
        !waterlily_vulkan_createFramebuffersSwapchain(context))
        return false;
    waterlily_engine_log(SUCCESS, "Recreated swapchain.");

    return true;
}

bool waterlily_vulkan_sync(waterlily_context_t *context)
{
    VkResult result = vkDeviceWaitIdle(context->gpu.logical);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to sync, code %d.", result);
        return false;
    }
    return true;
}

bool waterlily_vulkan_render(waterlily_context_t *context)
{
    VkFence waitFences[] = {
        context->commandBuffers.fences[context->currentFrame],
        context->commandBuffers.presentFence};
    vkWaitForFences(context->gpu.logical, 2, waitFences, true, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        context->gpu.logical, context->swapchain.handle, UINT64_MAX,
        context->commandBuffers.imageAvailableSemphores[context->currentFrame],
        nullptr, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        return waterlily_vulkan_recreateSwapchain(context);
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        waterlily_engine_log(
            ERROR, "Failed to acquire swapchain image, code %d.", result);
        return false;
    }

    vkResetFences(context->gpu.logical, 2, waitFences);

    vkResetCommandBuffer(context->commandBuffers.buffers[context->currentFrame],
                         0);
    if (!waterlily_vulkan_recordBufferCommand(context, imageIndex))
        return false;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores =
        &context->commandBuffers.imageAvailableSemphores[context->currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &context->commandBuffers.buffers[context->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores =
        &context->commandBuffers
             .renderFinishedSemaphores[context->currentFrame];

    result =
        vkQueueSubmit(context->queues.graphics.handle, 1, &submitInfo,
                      context->commandBuffers.fences[context->currentFrame]);
    if (result != VK_SUCCESS)
    {
        waterlily_engine_log(ERROR, "Failed to submit to the queue, code %d.",
                             result);
        return false;
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &context->commandBuffers
             .renderFinishedSemaphores[context->currentFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context->swapchain.handle;
    presentInfo.pImageIndices = &imageIndex;

    VkSwapchainPresentFenceInfoEXT presentFence = {0};
    presentFence.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT;
    presentFence.swapchainCount = 1;
    presentFence.pFences = &context->commandBuffers.presentFence;
    presentInfo.pNext = &presentFence;

    result = vkQueuePresentKHR(context->queues.graphics.handle, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        if (!waterlily_vulkan_recreateSwapchain(context))
            return false;
    }
    else if (result != VK_SUCCESS)
    {
        waterlily_engine_log(
            ERROR, "Failed to present swapchain image, code %d.", result);
        return false;
    }

    context->currentFrame =
        (context->currentFrame + 1) % WATERLILY_CONCURRENT_FRAMES;
    return true;
}

