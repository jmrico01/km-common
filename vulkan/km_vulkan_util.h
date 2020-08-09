#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "../km_array.h"
#include "../km_defines.h"
#include "../km_math.h"

#define SCOPED_VK_COMMAND_BUFFER(name, device, commandPool, queue) VkCommandBuffer name = \
BeginOneTimeCommands((device), (commandPool)); \
defer(EndOneTimeCommands((device), (commandPool), (queue), name));

struct RectCoordsNdc
{
    Vec2 pos;
    Vec2 size;
};

struct QueueFamilyInfo
{
    bool hasGraphicsFamily;
    uint32_t graphicsFamilyIndex;
    bool hasPresentFamily;
    uint32_t presentFamilyIndex;
    bool hasComputeFamily;
    uint32_t computeFamilyIndex;
};

struct VulkanBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanImage
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
};

VkCommandBuffer BeginOneTimeCommands(VkDevice device, VkCommandPool commandPool);
void EndOneTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

Vec2 ScreenPosToNdc(Vec2Int pos, Vec2Int screenSize);
RectCoordsNdc ToRectCoordsNdc(Vec2Int pos, Vec2Int size, Vec2Int screenSize);
RectCoordsNdc ToRectCoordsNdc(Vec2Int pos, Vec2Int size, Vec2 anchor, Vec2Int screenSize);

QueueFamilyInfo GetQueueFamilyInfo(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, LinearAllocator* allocator);

bool CreateShaderModule(const Array<uint8> code, VkDevice device, VkShaderModule* shaderModule);

bool CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags,
                        VkDevice device, VkPhysicalDevice physicalDevice, VulkanBuffer* buffer);
void DestroyVulkanBuffer(VkDevice device, VulkanBuffer* buffer);

void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer src, VkBuffer dst, VkDeviceSize size);

bool CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                 VkImage* image, VkDeviceMemory* imageMemory);

void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

bool CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                     VkImageView* imageView);

bool LoadVulkanImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue,
                     uint32 width, uint32 height, uint32 channels, const uint8* data, VulkanImage* image);
void DestroyVulkanImage(VkDevice device, VulkanImage* image);
