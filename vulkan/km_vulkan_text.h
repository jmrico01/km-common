#pragma once

#include "../km_array.h"
#include "../km_defines.h"
#include "../km_load_font.h"
#include "../km_math.h"
#include "../km_string.h"
#include "km_vulkan_core.h"
#include "km_vulkan_sprite.h"

//
// Shared utility for rendering text in Vulkan
//
// Your app's Vulkan state should have a VulkanTextPipeline object, with template parameter S
// specifying the maximum number of fonts (atlas textures) that your app will use
//
// Loaded by calling LoadTextPipelineWindow(...) and LoadTextPipelineSwapchain(...), in that order
// Unloaded by calling UnloadTextPipelineSwapchain(...) and UnloadTextPipelineWindow(...), in that order
// Register fonts (texture atlases) through RegisterFont(...), and keep each returned fontId to reference them
//
// Your app should have a VulkanTextRenderState, initially cleared through ResetTextRenderState(...)
// Then you can repeatedly call PushText(...) to push text render operations into the VulkanTextRenderState object
//
// When your app is filling the command buffer for rendering, you should call UploadAndSubmitTextDrawCommands(...)
// to draw all text recorded through PushText(...)
//

struct VulkanFontFace
{
    static const uint32 MAX_GLYPHS = 256;

    uint32 index;
	uint32 height;
	FixedArray<GlyphInfo, MAX_GLYPHS> glyphInfo;
};

template <uint32 S>
struct VulkanTextPipeline
{
    static const uint32 MAX_FONTS = S;
    static const uint32 MAX_INSTANCES = 4096;

    VulkanBuffer vertexBuffer;
    VulkanBuffer instanceBuffer;

    VkSampler atlasSampler;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    FixedArray<VulkanImage, MAX_FONTS> atlases;
    FixedArray<VkDescriptorSet, MAX_FONTS> descriptorSets;
};

struct VulkanTextInstanceData
{
    Vec3 pos;
    Vec2 size;
    Vec4 uvInfo;
    Vec4 color;
};

template <uint32 S>
struct VulkanTextRenderState
{
    using TextInstanceData = FixedArray<VulkanTextInstanceData, VulkanTextPipeline<S>::MAX_INSTANCES>;
    StaticArray<TextInstanceData, VulkanTextPipeline<S>::MAX_FONTS> textInstanceData;
};

uint32 GetTextWidth(const VulkanFontFace& fontFace, const_string text);

template <uint32 S>
void PushText(const VulkanFontFace& fontFace, const_string text, Vec2Int pos, float32 depth, Vec4 color,
              Vec2Int screenSize, VulkanTextRenderState<S>* renderState);

template <uint32 S>
void PushText(const VulkanFontFace& fontFace, const_string text, Vec2Int pos, float32 depth, float32 anchorX, Vec4 color,
              Vec2Int screenSize, VulkanTextRenderState<S>* renderState);

template <uint32 S>
void ResetTextRenderState(VulkanTextRenderState<S>* renderState);

template <uint32 S>
void UploadAndSubmitTextDrawCommands(VkDevice device, VkCommandBuffer commandBuffer,
                                     const VulkanTextPipeline<S>& textPipeline, const VulkanTextRenderState<S>& renderState,
                                     LinearAllocator* allocator);

template <uint32 S>
bool RegisterFont(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool,
                  VulkanTextPipeline<S>* textPipeline, const LoadFontFaceResult& fontFaceResult, VulkanFontFace* fontFace);

template <uint32 S>
bool LoadTextPipelineSwapchain(const VulkanWindow& window, const VulkanSwapchain& swapchain, LinearAllocator* allocator,
                               VulkanTextPipeline<S>* textPipeline);
template <uint32 S>
void UnloadTextPipelineSwapchain(VkDevice device, VulkanTextPipeline<S>* textPipeline);

template <uint32 S>
bool LoadTextPipelineWindow(const VulkanWindow& window, VkCommandPool commandPool, LinearAllocator* allocator,
                            VulkanTextPipeline<S>* textPipeline);
template <uint32 S>
void UnloadTextPipelineWindow(VkDevice device, VulkanTextPipeline<S>* textPipeline);
