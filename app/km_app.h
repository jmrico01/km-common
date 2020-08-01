#pragma once

// NOTE this header is a good candidate for putting in km_common or a new km_app lib or something

#include "../km_math.h"
#include "../vulkan/km_vulkan_core.h"
#include "km_input.h"

struct AppMemory
{
    bool initialized;
    LargeArray<uint8> permanent;
    LargeArray<uint8> transient;
};

struct AppWorkQueue;

#define APP_WORK_QUEUE_CALLBACK_FUNCTION(name) void name(uint32 threadIndex, AppWorkQueue* queue, void* data)
typedef APP_WORK_QUEUE_CALLBACK_FUNCTION(AppWorkQueueCallbackFunction);

struct AppWorkEntry
{
    AppWorkQueueCallbackFunction* callback;
    void* data;
};

struct AppWorkQueue
{
    volatile uint32 entriesTotal;
    volatile uint32 entriesComplete;

    volatile uint32 read;
    volatile uint32 write;
    AppWorkEntry entries[4 * 1024];
    HANDLE win32SemaphoreHandle;
};

uint32 GetCpuCount();
void CompleteAllWork(AppWorkQueue* queue, uint32 threadIndex);
bool TryAddWork(AppWorkQueue* queue, AppWorkQueueCallbackFunction* callback, void* data);

bool IsCursorLocked();
void LockCursor(bool locked);

struct AppAudio
{
};

#define APP_UPDATE_AND_RENDER_FUNCTION(name) bool name(const VulkanState& vulkanState, uint32_t swapchainImageIndex, \
const AppInput& input, float32 deltaTime, \
AppMemory* memory, AppAudio* audio, AppWorkQueue* queue)
typedef APP_UPDATE_AND_RENDER_FUNCTION(AppUpdateAndRenderFunction);

APP_UPDATE_AND_RENDER_FUNCTION(AppUpdateAndRender);

#define APP_LOAD_VULKAN_WINDOW_STATE_FUNCTION(name) bool name(const VulkanState& vulkanState, AppMemory* memory)
typedef APP_LOAD_VULKAN_WINDOW_STATE_FUNCTION(AppLoadVulkanWindowStateFunction);
APP_LOAD_VULKAN_WINDOW_STATE_FUNCTION(AppLoadVulkanWindowState);

#define APP_UNLOAD_VULKAN_WINDOW_STATE_FUNCTION(name) void name(const VulkanState& vulkanState, AppMemory* memory)
typedef APP_UNLOAD_VULKAN_WINDOW_STATE_FUNCTION(AppUnloadVulkanWindowStateFunction);
APP_UNLOAD_VULKAN_WINDOW_STATE_FUNCTION(AppUnloadVulkanWindowState);

#define APP_LOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(name) bool name(const VulkanState& vulkanState, AppMemory* memory)
typedef APP_LOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(AppLoadVulkanSwapchainStateFunction);
APP_LOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(AppLoadVulkanSwapchainState);

#define APP_UNLOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(name) void name(const VulkanState& vulkanState, AppMemory* memory)
typedef APP_UNLOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(AppUnloadVulkanSwapchainStateFunction);
APP_UNLOAD_VULKAN_SWAPCHAIN_STATE_FUNCTION(AppUnloadVulkanSwapchainState);
