#include "km_vulkan_sprite.h"

template <uint32 S>
void PushSprite(uint32 spriteIndex, Vec2Int pos, Vec2Int size, float32 depth, Vec4 color, Vec2Int screenSize,
                VulkanSpriteRenderState<S>* renderState)
{
    VulkanSpriteInstanceData* instanceData = renderState->spriteInstanceData[spriteIndex].Append();
    const RectCoordsNdc ndc = ToRectCoordsNdc(pos, size, screenSize);
    instanceData->pos = ToVec3(ndc.pos, depth);
    instanceData->size = ndc.size;
    instanceData->color = color;
}

template <uint32 S>
void PushSprite(uint32 spriteIndex, Vec2Int pos, Vec2Int size, float32 depth, Vec2 anchor, Vec4 color, Vec2Int screenSize,
                VulkanSpriteRenderState<S>* renderState)
{
    VulkanSpriteInstanceData* instanceData = renderState->spriteInstanceData[spriteIndex].Append();
    const RectCoordsNdc ndc = ToRectCoordsNdc(pos, size, anchor, screenSize);
    instanceData->pos = ToVec3(ndc.pos, depth);
    instanceData->size = ndc.size;
    instanceData->color = color;
}

template <uint32 S>
void ResetSpriteRenderState(VulkanSpriteRenderState<S>* renderState)
{
    for (uint32 i = 0; i < renderState->spriteInstanceData.SIZE; i++) {
        renderState->spriteInstanceData[i].Clear();
    }
}

template <uint32 S>
void UploadAndSubmitSpriteDrawCommands(VkDevice device, VkCommandBuffer commandBuffer,
                                       const VulkanSpritePipeline<S>& spritePipeline, const VulkanSpriteRenderState<S>& renderState,
                                       LinearAllocator* allocator)
{
    Array<uint32> spriteNumInstances = allocator->NewArray<uint32>(spritePipeline.sprites.size);
    uint32 totalNumInstances = 0;
    for (uint32 i = 0; i < spriteNumInstances.size; i++) {
        spriteNumInstances[i] = renderState.spriteInstanceData[i].size;
        totalNumInstances += spriteNumInstances[i];
    }

    if (totalNumInstances > spritePipeline.MAX_INSTANCES) {
        LOG_ERROR("Too many sprite instances: %lu, max %lu\n", totalNumInstances, spritePipeline.MAX_INSTANCES);
        // TODO what to do here?
        DEBUG_PANIC("too many sprites!\n");
    }

    Array<VulkanSpriteInstanceData> instanceData = allocator->NewArray<VulkanSpriteInstanceData>(totalNumInstances);
    uint32 instanceOffset = 0;
    for (uint32 i = 0; i < spritePipeline.sprites.size; i++) {
        const uint32 instances = renderState.spriteInstanceData[i].size;
        MemCopy(instanceData.data + instanceOffset,
                renderState.spriteInstanceData[i].data, instances * sizeof(VulkanSpriteInstanceData));
        instanceOffset += instances;
    }

    if (instanceData.size > 0) {
        void* data;
        const uint32 bufferSize = instanceData.size * sizeof(VulkanSpriteInstanceData);
        vkMapMemory(device, spritePipeline.instanceBuffer.memory, 0, bufferSize, 0, &data);
        MemCopy(data, instanceData.data, bufferSize);
        vkUnmapMemory(device, spritePipeline.instanceBuffer.memory);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.pipeline);

    const VkBuffer vertexBuffers[] = { spritePipeline.vertexBuffer.buffer, spritePipeline.instanceBuffer.buffer };
    const VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, C_ARRAY_LENGTH(vertexBuffers), vertexBuffers, offsets);

    uint32 startInstance = 0;
    for (uint32 i = 0; i < spriteNumInstances.size; i++) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.pipelineLayout, 0, 1,
                                &spritePipeline.descriptorSets[i], 0, nullptr);

        if (spriteNumInstances[i] > 0) {
            vkCmdDraw(commandBuffer, 6, spriteNumInstances[i], 0, startInstance);
            startInstance += spriteNumInstances[i];
        }
    }
}

template <uint32 S>
bool RegisterSprite(VkDevice device, VulkanSpritePipeline<S>* spritePipeline, VulkanImage sprite, uint32* spriteIndex)
{
    const uint32 index = spritePipeline->sprites.size;
    if (index >= spritePipeline->MAX_SPRITES) {
        return false;
    }
    *spriteIndex = index;

    VulkanImage* newSprite = spritePipeline->sprites.Append(sprite);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = spritePipeline->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &spritePipeline->descriptorSetLayout;

    VkDescriptorSet* newDescriptorSet = spritePipeline->descriptorSets.Append();
    if (vkAllocateDescriptorSets(device, &allocInfo, newDescriptorSet) != VK_SUCCESS) {
        LOG_ERROR("vkAllocateDescriptorSets failed\n");
        return false;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = newSprite->view;
    imageInfo.sampler = spritePipeline->spriteSampler;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = *newDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

    return true;
}

template <uint32 S>
bool LoadSpritePipelineSwapchain(const VulkanWindow& window, const VulkanSwapchain& swapchain, LinearAllocator* allocator,
                                 VulkanSpritePipeline<S>* spritePipeline)
{
    const Array<uint8> vertShaderCode = LoadEntireFile(ToString("data/shaders/sprite.vert.spv"), allocator);
    if (vertShaderCode.data == nullptr) {
        LOG_ERROR("Failed to load vertex shader code\n");
        return false;
    }
    const Array<uint8> fragShaderCode = LoadEntireFile(ToString("data/shaders/sprite.frag.spv"), allocator);
    if (fragShaderCode.data == nullptr) {
        LOG_ERROR("Failed to load fragment shader code\n");
        return false;
    }

    VkShaderModule vertShaderModule;
    if (!CreateShaderModule(vertShaderCode, window.device, &vertShaderModule)) {
        LOG_ERROR("Failed to create vertex shader module\n");
        return false;
    }
    defer(vkDestroyShaderModule(window.device, vertShaderModule, nullptr));

    VkShaderModule fragShaderModule;
    if (!CreateShaderModule(fragShaderCode, window.device, &fragShaderModule)) {
        LOG_ERROR("Failed to create fragment shader module\n");
        return false;
    }
    defer(vkDestroyShaderModule(window.device, fragShaderModule, nullptr));

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";
    // vertShaderStageCreateInfo.pSpecializationInfo is useful for setting shader constants

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

    VkVertexInputBindingDescription bindingDescriptions[2] = {};
    VkVertexInputAttributeDescription attributeDescriptions[5] = {};

    // Per-vertex attribute bindings
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(VulkanSpriteVertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(VulkanSpriteVertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(VulkanSpriteVertex, uv);

    // Per-instance attribute bindings
    bindingDescriptions[1].binding = 1;
    bindingDescriptions[1].stride = sizeof(VulkanSpriteInstanceData);
    bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributeDescriptions[2].binding = 1;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(VulkanSpriteInstanceData, pos);

    attributeDescriptions[3].binding = 1;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(VulkanSpriteInstanceData, size);

    attributeDescriptions[4].binding = 1;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(VulkanSpriteInstanceData, color);

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = C_ARRAY_LENGTH(bindingDescriptions);
    vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = C_ARRAY_LENGTH(attributeDescriptions);
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float32)swapchain.extent.width;
    viewport.height = (float32)swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
    colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingCreateInfo.attachmentCount = 1;
    colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendingCreateInfo.blendConstants[0] = 0.0f;
    colorBlendingCreateInfo.blendConstants[1] = 0.0f;
    colorBlendingCreateInfo.blendConstants[2] = 0.0f;
    colorBlendingCreateInfo.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.minDepthBounds = 0.0f; // disabled
    depthStencilCreateInfo.maxDepthBounds = 1.0f; // disabled
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &spritePipeline->descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(window.device, &pipelineLayoutCreateInfo, nullptr,
                               &spritePipeline->pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("vkCreatePipelineLayout failed\n");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = C_ARRAY_LENGTH(shaderStages);
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = spritePipeline->pipelineLayout;
    pipelineCreateInfo.renderPass = swapchain.renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(window.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr,
                                  &spritePipeline->pipeline) != VK_SUCCESS) {
        LOG_ERROR("vkCreateGraphicsPipeline failed\n");
        return false;
    }

    return true;
}

template <uint32 S>
void UnloadSpritePipelineSwapchain(VkDevice device, VulkanSpritePipeline<S>* spritePipeline)
{
    vkDestroyPipeline(device, spritePipeline->pipeline, nullptr);
    vkDestroyPipelineLayout(device, spritePipeline->pipelineLayout, nullptr);
}

template <uint32 S>
bool LoadSpritePipelineWindow(const VulkanWindow& window, VkCommandPool commandPool, LinearAllocator* allocator,
                              VulkanSpritePipeline<S>* spritePipeline)
{
    UNREFERENCED_PARAMETER(allocator);

    spritePipeline->sprites.Clear();
    spritePipeline->descriptorSets.Clear();

    // Create vertex buffer
    {
        const VulkanSpriteVertex VERTICES[] = {
            { { 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { 1.0f, 1.0f }, { 1.0f, 1.0f } },

            { { 1.0f, 1.0f }, { 1.0f, 1.0f } },
            { { 0.0f, 1.0f }, { 0.0f, 1.0f } },
            { { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        };

        const VkDeviceSize vertexBufferSize = C_ARRAY_LENGTH(VERTICES) * sizeof(VulkanSpriteVertex);

        VulkanBuffer stagingBuffer;
        if (!CreateVulkanBuffer(vertexBufferSize,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                window.device, window.physicalDevice, &stagingBuffer)) {
            LOG_ERROR("CreateBuffer failed for staging buffer\n");
            return false;
        }

        // Copy vertex data from CPU into memory-mapped staging buffer
        void* data;
        vkMapMemory(window.device, stagingBuffer.memory, 0, vertexBufferSize, 0, &data);

        MemCopy(data, VERTICES, vertexBufferSize);

        vkUnmapMemory(window.device, stagingBuffer.memory);

        if (!CreateVulkanBuffer(vertexBufferSize,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                window.device, window.physicalDevice,
                                &spritePipeline->vertexBuffer)) {
            LOG_ERROR("CreateBuffer failed for vertex buffer\n");
            return false;
        }

        // Copy vertex data from staging buffer into GPU vertex buffer
        VkCommandBuffer commandBuffer = BeginOneTimeCommands(window.device, commandPool);
        CopyBuffer(commandBuffer, stagingBuffer.buffer, spritePipeline->vertexBuffer.buffer, vertexBufferSize);
        EndOneTimeCommands(window.device, commandPool, window.graphicsQueue, commandBuffer);

        DestroyVulkanBuffer(window.device, &stagingBuffer);
    }

    // Create instance buffer
    {
        const VkDeviceSize bufferSize = spritePipeline->MAX_INSTANCES * sizeof(VulkanSpriteInstanceData);

        if (!CreateVulkanBuffer(bufferSize,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                window.device, window.physicalDevice, &spritePipeline->instanceBuffer)) {
            LOG_ERROR("CreateBuffer failed for instance buffer\n");
            return false;
        }
    }

    // Create texture sampler
    {
        VkSamplerCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1.0f;
        createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        createInfo.unnormalizedCoordinates = VK_FALSE;
        createInfo.compareEnable = VK_FALSE;
        createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.mipLodBias = 0.0f;
        createInfo.minLod = 0.0f;
        createInfo.maxLod = 0.0f;

        if (vkCreateSampler(window.device, &createInfo, nullptr, &spritePipeline->spriteSampler) != VK_SUCCESS) {
            LOG_ERROR("vkCreateSampler failed\n");
            return false;
        }
    }

    // Create descriptor set layout
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(window.device, &layoutCreateInfo, nullptr,
                                        &spritePipeline->descriptorSetLayout) != VK_SUCCESS) {
            LOG_ERROR("vkCreateDescriptorSetLayout failed\n");
            return false;
        }
    }

    // Create descriptor pool
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = spritePipeline->MAX_SPRITES;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = spritePipeline->MAX_SPRITES;

        if (vkCreateDescriptorPool(window.device, &poolInfo, nullptr, &spritePipeline->descriptorPool) != VK_SUCCESS) {
            LOG_ERROR("vkCreateDescriptorPool failed\n");
            return false;
        }
    }

    return true;
}

template <uint32 S>
void UnloadSpritePipelineWindow(VkDevice device, VulkanSpritePipeline<S>* spritePipeline)
{
    vkDestroyDescriptorPool(device, spritePipeline->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, spritePipeline->descriptorSetLayout, nullptr);

    vkDestroySampler(device, spritePipeline->spriteSampler, nullptr);

    for (uint32 i = 0; i < spritePipeline->sprites.size; i++) {
        DestroyVulkanImage(device, &spritePipeline->sprites[i]);
    }

    DestroyVulkanBuffer(device, &spritePipeline->instanceBuffer);
    DestroyVulkanBuffer(device, &spritePipeline->vertexBuffer);
}
