#include <internal/files.h>
#include <internal/logging.h>
#include <internal/vulkan.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_wayland.h>

static struct waterlily_vulkan_context context = {0};

static void createCommandBuffers(void)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = context.gpu.graphicsQueue.index;

    VkResult result = vkCreateCommandPool(
        context.gpu.logical, &poolInfo, nullptr, &context.commandBuffers.pool);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create command pool, code %d.", result);
    waterlily_log(SUCCESS, "Created command pool.");

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.commandBuffers.pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = WATERLILY_CONCURRENT_FRAMES;

    result = vkAllocateCommandBuffers(context.gpu.logical, &allocInfo,
                                      context.commandBuffers.buffers);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create command buffers, code %d.", result);
    waterlily_log(SUCCESS, "Created command buffers.");
}

static void createSyncDevices(void)
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateFence(context.gpu.logical, &fenceInfo, nullptr,
                                    &context.commandBuffers.presentFence);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create present fence, code %d.", result);

    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        result = vkCreateSemaphore(
            context.gpu.logical, &semaphoreInfo, nullptr,
            &context.commandBuffers.imageAvailableSemphores[i]);
        if (result != VK_SUCCESS)
            waterlily_report(
                "Failed to create image available semaphore %zu, code %d.", i,
                result);

        result = vkCreateSemaphore(
            context.gpu.logical, &semaphoreInfo, nullptr,
            &context.commandBuffers.renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
            waterlily_report(
                "Failed to create render finished semaphore %zu, code %d.", i,
                result);

        result = vkCreateFence(context.gpu.logical, &fenceInfo, nullptr,
                               &context.commandBuffers.fences[i]);
        if (result != VK_SUCCESS)
            waterlily_report("Failed to create fence %zu, code %d.", i, result);
    }
}
static uint32_t scoreDevice(const char *const *const extensions, size_t count)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(context.gpu.physical, &properties);
    vkGetPhysicalDeviceFeatures(context.gpu.physical, &features);

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
    vkEnumerateDeviceExtensionProperties(context.gpu.physical, nullptr,
                                         &extensionCount, nullptr);
    VkExtensionProperties foundExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(context.gpu.physical, nullptr,
                                         &extensionCount, foundExtensions);

    size_t requiredCount = count;
    size_t foundCount = 0;
    for (size_t i = 0; i < extensionCount; ++i)
    {
        if (foundCount == requiredCount)
            break;

        VkExtensionProperties extension = foundExtensions[i];
        waterlily_log(INFO, "Found extension '%s'.", extension.extensionName);

        for (size_t j = 0; j < requiredCount; ++j)
            if (strcmp(extension.extensionName, extensions[j]) == 0)
            {
                foundCount++;
                waterlily_log(SUCCESS, "Found '%s' device extension.",
                              extension.extensionName);
                break;
            }
    }

    if (foundCount != requiredCount)
        waterlily_report("Failed to find all required device extensions.");

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context.gpu.physical,
                                             &queueFamilyCount, nullptr);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(context.gpu.physical,
                                             &queueFamilyCount, queueFamilies);

    bool foundGraphicsQueue = false, foundPresentQueue = false;
    for (size_t i = 0; i < queueFamilyCount; i++)
    {
        if (foundGraphicsQueue && foundPresentQueue)
            break;

        VkQueueFamilyProperties family = queueFamilies[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            context.gpu.graphicsQueue.index = i;
            foundGraphicsQueue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            context.gpu.physical, i, context.surface.handle, &presentSupport);
        if (presentSupport)
        {
            context.gpu.presentQueue.index = i;
            foundPresentQueue = true;
        }
    }

    if (!foundGraphicsQueue || !foundPresentQueue)
        waterlily_report("Failed to find required queue.");
    return score;
}

static void getPhysicalGPU(const char *const *const extensions, size_t count)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(context.instance, &deviceCount, devices);

    uint32_t bestScore = 0;
    for (size_t i = 0; i < deviceCount; i++)
    {
        context.gpu.physical = devices[i];
        uint32_t score = scoreDevice(extensions, count);
        if (score > bestScore)
            bestScore = score;
    }

    if (bestScore == 0)
        waterlily_report("Failed to find suitable Vulkan device.");
    waterlily_log(SUCCESS, "Found suitable Vulkan device.");
}

static void createLogicalGPU(void)
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {{0}, {0}};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = context.gpu.graphicsQueue.index;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &priority;

    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = context.gpu.presentQueue.index;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &priority;

    // Layers for logical devices no longer need to be set in newer
    // implementations.
    VkDeviceCreateInfo logicalDeviceCreateInfo = {0};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    logicalDeviceCreateInfo.queueCreateInfoCount =
        context.gpu.graphicsQueue.index == context.gpu.presentQueue.index ? 1
                                                                          : 2;
    logicalDeviceCreateInfo.pEnabledFeatures = nullptr;
    logicalDeviceCreateInfo.pNext = &(VkPhysicalDeviceFeatures2){
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext =
            &(VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR){
                .sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
                .swapchainMaintenance1 = true,
            },
    };

    const char *const extensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_swapchain_maintenance1",
    };

    logicalDeviceCreateInfo.enabledExtensionCount =
        sizeof(extensions) / sizeof(char *);
    logicalDeviceCreateInfo.ppEnabledExtensionNames = extensions;

    getPhysicalGPU(extensions, logicalDeviceCreateInfo.enabledExtensionCount);

    VkResult code =
        vkCreateDevice(context.gpu.physical, &logicalDeviceCreateInfo, nullptr,
                       &context.gpu.logical);
    if (code != VK_SUCCESS)
        waterlily_report("Failed to create logical device. Code: %d.", code);
    waterlily_log(SUCCESS, "Created logical device.");

    vkGetDeviceQueue(context.gpu.logical, context.gpu.graphicsQueue.index, 0,
                     &context.gpu.graphicsQueue.handle);
    vkGetDeviceQueue(context.gpu.logical, context.gpu.presentQueue.index, 0,
                     &context.gpu.presentQueue.handle);
    waterlily_log(INFO, "Created device data queues.");
}

static void createPipelineLayout(void)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result =
        vkCreatePipelineLayout(context.gpu.logical, &pipelineLayoutInfo,
                               nullptr, &context.pipeline.layout);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create pipeline layout. Code: %d.", result);
    waterlily_log(SUCCESS, "Created pipeline layout.");
}

static void createPipelineRenderpass(void)
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
    colorAttachment.format = context.surface.format.format;
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

    VkResult result = vkCreateRenderPass(context.gpu.logical, &renderPassInfo,
                                         nullptr, &context.pipeline.renderpass);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create renderpass. Code: %d.", result);
    waterlily_log(SUCCESS, "Created renderpass.");
}

static void createShaderStages(VkPipelineShaderStageCreateInfo *storage)
{
    waterlily_file_t shaderFile = {
        .name = "shaders",
        .type = WATERLILY_SHADER_FILE,
    };
    waterlily_readFile(&shaderFile);

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
                waterlily_report("Unknown shader stage index %d.", i);
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
            context.gpu.logical, &moduleCreate, nullptr, &info->module);
        if (result != VK_SUCCESS)
            waterlily_report("Failed to create shader module, code %d.",
                             result);
    }
}

static void createPipeline(void)
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo stages[WATERLILY_SHADER_STAGES];
    createShaderStages(stages);
    pipelineInfo.pStages = stages;
    pipelineInfo.stageCount = WATERLILY_SHADER_STAGES;

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

    createPipelineLayout();
    createPipelineRenderpass();
    pipelineInfo.layout = context.pipeline.layout;
    pipelineInfo.renderPass = context.pipeline.renderpass;

    VkResult result = vkCreateGraphicsPipelines(context.gpu.logical, nullptr, 1,
                                                &pipelineInfo, nullptr,
                                                &context.pipeline.handle);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create graphics pipeline. Code: %d.",
                         result);
    waterlily_log(SUCCESS, "Created graphics pipeline.");

    for (size_t i = 0; i < WATERLILY_SHADER_STAGES; ++i)
        vkDestroyShaderModule(context.gpu.logical, stages[i].module, nullptr);
}

static void createSurface(struct waterlily_window_context *window)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = window->display;
    createInfo.surface = window->surface;

    VkResult result = vkCreateWaylandSurfaceKHR(
        context.instance, &createInfo, nullptr, &context.surface.handle);
    if (result != VK_SUCCESS)
        waterlily_report(
            "Failed to create Vulkan-Wayland interop surface. Code: %d.",
            result);
    waterlily_log(SUCCESS, "Created Vulkan-Wayland interop surface.");
}

static void getSurfaceCapabilities(void)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context.gpu.physical, context.surface.handle, &context.surface.info);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to get surface capabilities, code %d.\n",
                         result);
    waterlily_log(SUCCESS, "Got surface capabilities.");
}

static void createSwapchain(void)
{
    context.swapchain.imageCount = context.surface.info.minImageCount + 1;
    if (context.surface.info.maxImageCount > 0 &&
        context.swapchain.imageCount > context.surface.info.maxImageCount)
        context.swapchain.imageCount = context.surface.info.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context.surface.handle;
    createInfo.minImageCount = context.swapchain.imageCount;
    createInfo.imageFormat = context.surface.format.format;
    createInfo.imageColorSpace = context.surface.format.colorSpace;
    createInfo.imageExtent = context.surface.extent;
    createInfo.presentMode = context.surface.mode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = context.surface.info.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    if (context.gpu.graphicsQueue.index != context.gpu.presentQueue.index)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t indices[2] = {context.gpu.graphicsQueue.index,
                               context.gpu.presentQueue.index};
        createInfo.pQueueFamilyIndices = indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult code = vkCreateSwapchainKHR(context.gpu.logical, &createInfo,
                                         nullptr, &context.swapchain.handle);
    if (code != VK_SUCCESS)
        waterlily_report("Failed to create swapchain, code %d.", code);
}

static void createFramebuffers(void)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = context.pipeline.renderpass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = context.surface.extent.width;
    framebufferInfo.height = context.surface.extent.height;
    framebufferInfo.layers = 1;

    context.swapchain.framebuffers =
        malloc(sizeof(VkFramebuffer) * context.swapchain.imageCount);

    for (size_t i = 0; i < context.swapchain.imageCount; ++i)
    {
        framebufferInfo.pAttachments = &context.swapchain.images[i];
        VkResult result =
            vkCreateFramebuffer(context.gpu.logical, &framebufferInfo, nullptr,
                                &context.swapchain.framebuffers[i]);
        if (result != VK_SUCCESS)
            waterlily_report("Failed to create framebuffer %zu, code %d.", i,
                             result);
        waterlily_log(SUCCESS, "Created framebuffer %zu.", i);
    }
}

static void partitionSwapchain(void)
{
    VkImage rawImages[context.swapchain.imageCount];
    VkResult code =
        vkGetSwapchainImagesKHR(context.gpu.logical, context.swapchain.handle,
                                &context.swapchain.imageCount, rawImages);
    if (code != VK_SUCCESS)
        waterlily_report("Failed to get swapchain images, code %d.", code);

    VkImageViewCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageCreateInfo.format = context.surface.format.format;
    imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCreateInfo.subresourceRange.levelCount = 1;
    imageCreateInfo.subresourceRange.layerCount = 1;

    context.swapchain.images =
        malloc(sizeof(VkImageView) * context.swapchain.imageCount);
    for (size_t i = 0; i < context.swapchain.imageCount; i++)
    {
        imageCreateInfo.image = rawImages[i];
        VkResult result =
            vkCreateImageView(context.gpu.logical, &imageCreateInfo, nullptr,
                              &context.swapchain.images[i]);
        if (result != VK_SUCCESS)
            waterlily_report("Failed to create image view %zu, code %d.", i,
                             result);
        waterlily_log(SUCCESS, "Created image view %zu.", i);
    }
}

static void recordCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(
        context.commandBuffers.buffers[context.currentFrame], &beginInfo);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to begin command buffer, code %d.", result);

    VkViewport viewport = {0};
    viewport.width = (float)context.surface.extent.width;
    viewport.height = (float)context.surface.extent.height;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent.width = context.surface.extent.width;
    scissor.extent.height = context.surface.extent.height;

    vkCmdSetViewport(context.commandBuffers.buffers[context.currentFrame], 0, 1,
                     &viewport);
    vkCmdSetScissor(context.commandBuffers.buffers[context.currentFrame], 0, 1,
                    &scissor);

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context.pipeline.renderpass;
    renderPassInfo.framebuffer = context.swapchain.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = context.surface.extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(context.commandBuffers.buffers[context.currentFrame],
                         &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(context.commandBuffers.buffers[context.currentFrame],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline.handle);
    vkCmdDraw(context.commandBuffers.buffers[context.currentFrame], 3, 1, 0, 0);
    vkCmdEndRenderPass(context.commandBuffers.buffers[context.currentFrame]);

    result = vkEndCommandBuffer(
        context.commandBuffers.buffers[context.currentFrame]);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to end command buffer, code %d.", result);
}

// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#16659263
static inline uint32_t clamp(uint32_t d, uint32_t min, uint32_t max)
{
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

static void getSurfaceExtent(void)
{
    if (context.surface.info.currentExtent.width != UINT32_MAX)
    {
        context.surface.extent = context.surface.info.currentExtent;
        return;
    }

    context.surface.extent.width = clamp(
        context.surface.extent.width, context.surface.info.minImageExtent.width,
        context.surface.info.maxImageExtent.width);
    context.surface.extent.height =
        clamp(context.surface.extent.height,
              context.surface.info.minImageExtent.height,
              context.surface.info.maxImageExtent.height);
}

static void getSurfaceFormat(void)
{
    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context.gpu.physical, context.surface.handle, &formatCount, nullptr);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to enumerate surface formats (1), code %d.",
                         result);

    VkSurfaceFormatKHR formats[formatCount];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context.gpu.physical, context.surface.handle, &formatCount, formats);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to enumerate surface formats (2), code %d.",
                         result);

    for (size_t i = 0; i < formatCount; ++i)
    {
        VkSurfaceFormatKHR currentFormat = formats[i];
        // This is the best combination. If not available, we'll just
        // select the first provided colorspace.
        if (currentFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            context.surface.format = currentFormat;
            waterlily_log(SUCCESS, "Found optimal surface format.");
            return;
        }
    }

    waterlily_report(
        "Could not find the optimal colorspace. Falling back on %dx%d.",
        formats[0].format, formats[0].colorSpace);
    context.surface.format = formats[0];
}

static void getSurfaceMode(void)
{
    uint32_t modeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        context.gpu.physical, context.surface.handle, &modeCount, nullptr);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to enumerator present modes (1), code %d.",
                         result);

    VkPresentModeKHR modes[modeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        context.gpu.physical, context.surface.handle, &modeCount, modes);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to enumerator present modes (2), code %d.",
                         result);

    for (size_t i = 0; i < modeCount; ++i)
    {
        // This is the best mode. If not available, we'll just select
        // VK_PRESENT_MODE_FIFO_KHR.
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            context.surface.mode = modes[i];
            waterlily_log(SUCCESS, "Found optimal present mode.");
            return;
        }
    }

    waterlily_report("Failed to find optimal present mode.");
    context.surface.mode = VK_PRESENT_MODE_FIFO_KHR;
}

static void syncGPU(void)
{
    VkResult result = vkDeviceWaitIdle(context.gpu.logical);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to sync, code %d.", result);
}

static void destroySwapchain()
{
    for (size_t i = 0; i < context.swapchain.imageCount; ++i)
    {
        vkDestroyFramebuffer(context.gpu.logical,
                             context.swapchain.framebuffers[i], nullptr);
        vkDestroyImageView(context.gpu.logical, context.swapchain.images[i],
                           nullptr);
    }
    vkDestroySwapchainKHR(context.gpu.logical, context.swapchain.handle,
                          nullptr);
    free(context.swapchain.framebuffers);
    free(context.swapchain.images);
}

void recreateSwapchain(void)
{
    syncGPU();
    destroySwapchain();

    createSwapchain();
    partitionSwapchain();
    createFramebuffers();
    waterlily_log(SUCCESS, "Recreated swapchain.");
}

void waterlily_renderFrame(void)
{
    VkFence waitFences[] = {context.commandBuffers.fences[context.currentFrame],
                            context.commandBuffers.presentFence};
    vkWaitForFences(context.gpu.logical, 2, waitFences, true, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        context.gpu.logical, context.swapchain.handle, UINT64_MAX,
        context.commandBuffers.imageAvailableSemphores[context.currentFrame],
        nullptr, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        recreateSwapchain();
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        waterlily_report("Failed to acquire swapchain image, code %d.", result);

    vkResetFences(context.gpu.logical, 2, waitFences);

    vkResetCommandBuffer(context.commandBuffers.buffers[context.currentFrame],
                         0);
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores =
        &context.commandBuffers.imageAvailableSemphores[context.currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &context.commandBuffers.buffers[context.currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores =
        &context.commandBuffers.renderFinishedSemaphores[context.currentFrame];

    result = vkQueueSubmit(context.gpu.graphicsQueue.handle, 1, &submitInfo,
                           context.commandBuffers.fences[context.currentFrame]);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to submit to the queue, code %d.", result);

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &context.commandBuffers.renderFinishedSemaphores[context.currentFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context.swapchain.handle;
    presentInfo.pImageIndices = &imageIndex;

    VkSwapchainPresentFenceInfoEXT presentFence = {0};
    presentFence.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT;
    presentFence.swapchainCount = 1;
    presentFence.pFences = &context.commandBuffers.presentFence;
    presentInfo.pNext = &presentFence;

    result = vkQueuePresentKHR(context.gpu.graphicsQueue.handle, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        recreateSwapchain();
    else if (result != VK_SUCCESS)
        waterlily_report("Failed to present swapchain image, code %d.", result);

    context.currentFrame =
        (context.currentFrame + 1) % WATERLILY_CONCURRENT_FRAMES;
}

#if BUILD_TYPE == 0
static VkBool32
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
              VkDebugUtilsMessageTypeFlagsEXT,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    waterlily_report("Vulkan error recieved: '%s'.", pCallbackData->pMessage);
}
#endif

struct waterlily_vulkan_context *
waterlily_createVulkanContext(struct waterlily_window_context *window)
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
        waterlily_log(INFO, "Found instance extension '%s'.",
                      properties->extensionName);
        for (size_t j = 0; j < instanceInfo.enabledExtensionCount; ++j)
        {
            if (strcmp(properties->extensionName, extensions[j]) == 0)
            {
                waterlily_log(SUCCESS,
                              "Found requested instance extension '%s'.",
                              properties->extensionName);
                foundProvidedExtensions++;
                break;
            }
        }
    }

    if (foundProvidedExtensions != instanceInfo.enabledExtensionCount)
        waterlily_report(
            "Failed to find the required amount of instance extensions.");
    waterlily_log(SUCCESS, "Found all required instance extensions.");

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
        vkCreateInstance(&instanceInfo, nullptr, &context.instance);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create Vulkan instance. Code: %d.", result);
    waterlily_log(SUCCESS, "Created Vulkan instance.");

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

    auto debugCreator =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkCreateDebugUtilsMessengerEXT");
    result = debugCreator(context.instance, &createInfo, nullptr,
                          &context.debugMessenger);
    if (result != VK_SUCCESS)
        waterlily_report("Failed to create Vulkan debug messenger, code %d.",
                         result);
    waterlily_log(SUCCESS, "Created Vulkan debug messenger.");
#endif

    createSurface(window);
    createLogicalGPU();
    getSurfaceFormat();
    getSurfaceMode();
    getSurfaceCapabilities();
    getSurfaceExtent();
    createPipeline();
    createSwapchain();
    createCommandBuffers();
    createSyncDevices();

    return &context;
}

void waterlily_vulkan_destroy(void)
{
#if BUILD_TYPE == 0
    auto debugDestroy =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkDestroyDebugUtilsMessengerEXT");
    debugDestroy(context.instance, context.debugMessenger, nullptr);
#endif

    vkDestroyCommandPool(context.gpu.logical, context.commandBuffers.pool,
                         nullptr);

    vkDestroyFence(context.gpu.logical, context.commandBuffers.presentFence,
                   nullptr);
    for (size_t i = 0; i < WATERLILY_CONCURRENT_FRAMES; ++i)
    {
        vkDestroySemaphore(context.gpu.logical,
                           context.commandBuffers.imageAvailableSemphores[i],
                           nullptr);
        vkDestroySemaphore(context.gpu.logical,
                           context.commandBuffers.renderFinishedSemaphores[i],
                           nullptr);
        vkDestroyFence(context.gpu.logical, context.commandBuffers.fences[i],
                       nullptr);
    }

    vkDestroyRenderPass(context.gpu.logical, context.pipeline.renderpass,
                        nullptr);
    vkDestroyPipelineLayout(context.gpu.logical, context.pipeline.layout,
                            nullptr);
    vkDestroyPipeline(context.gpu.logical, context.pipeline.handle, nullptr);

    destroySwapchain();

    syncGPU();
    vkDestroyDevice(context.gpu.logical, nullptr);
    vkDestroySurfaceKHR(context.instance, context.surface.handle, nullptr);
    vkDestroyInstance(context.instance, nullptr);
}

