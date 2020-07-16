#pragma once

#include "../km_defines.h"
#include "../km_math.h"
#include "km_vulkan_core.h"

//
// Shared utility for rendering sprites in Vulkan
//
// Your app's Vulkan state should have a VulkanSpritePipeline object, with template parameter S
// specifying the maximum number of sprites that you app will use
//
// Loaded by calling LoadSpritePipelineWindow(...) and LoadSpritePipelineSwapchain(...), in that order
// Unloaded by calling UnloadSpritePipelineSwapchain(...) and UnloadSpritePipelineWindow(...), in that order
// Register sprites through RegisterSprite(...), and keep each returned spriteIndex to reference them
//
// Your app should have a VulkanSpriteRenderState, initially cleared through ResetSpriteRenderState(...)
// Then you can repeatedly call PushSprite(...) to push sprite render operations into the VulkanSpriteRenderState object
//
// When your app is filling the command buffer for rendering, you should call UploadAndSubmitTextDrawCommands(...)
// to draw all sprites recorded through PushSprite(...)
//

template <uint32 S>
struct VulkanSpritePipeline
{
    static const uint32 MAX_SPRITES = S;
    static const uint32 MAX_INSTANCES = 64;

    VulkanBuffer vertexBuffer;
    VulkanBuffer instanceBuffer;

    VkSampler spriteSampler;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    FixedArray<VulkanImage, MAX_SPRITES> sprites;
    FixedArray<VkDescriptorSet, MAX_SPRITES> descriptorSets;
};

struct VulkanSpriteVertex
{
    Vec2 pos;
    Vec2 uv;
};

struct VulkanSpriteInstanceData
{
    Vec3 pos;
    Vec2 size;
};

template <uint32 S>
struct VulkanSpriteRenderState
{
    using SpriteInstanceData = FixedArray<VulkanSpriteInstanceData, VulkanSpritePipeline<S>::MAX_INSTANCES>;
    StaticArray<SpriteInstanceData, VulkanSpritePipeline<S>::MAX_SPRITES> spriteInstanceData;
};

template <uint32 S>
void PushSprite(uint32 spriteIndex, Vec2Int pos, Vec2Int size, float32 depth, Vec2Int screenSize,
                VulkanSpriteRenderState<S>* renderState);

template <uint32 S>
void ResetSpriteRenderState(VulkanSpriteRenderState<S>* renderState);

template <uint32 S>
void UploadAndSubmitSpriteDrawCommands(VkDevice device, VkCommandBuffer commandBuffer,
                                       const VulkanSpritePipeline<S>& spritePipeline, const VulkanSpriteRenderState<S>& renderState,
                                       LinearAllocator* allocator);

template <uint32 S>
bool RegisterSprite(VkDevice device, VulkanSpritePipeline<S>* spritePipeline, VulkanImage sprite, uint32* spriteIndex);

template <uint32 S>
bool LoadSpritePipelineSwapchain(const VulkanWindow& window, const VulkanSwapchain& swapchain, LinearAllocator* allocator,
                                 VulkanSpritePipeline<S>* spritePipeline);
template <uint32 S>
void UnloadSpritePipelineSwapchain(VkDevice device, VulkanSpritePipeline<S>* spritePipeline);

template <uint32 S>
bool LoadSpritePipelineWindow(const VulkanWindow& window, VkCommandPool commandPool, LinearAllocator* allocator,
                              VulkanSpritePipeline<S>* spritePipeline);
template <uint32 S>
void UnloadSpritePipelineWindow(VkDevice device, VulkanSpritePipeline<S>* spritePipeline);
