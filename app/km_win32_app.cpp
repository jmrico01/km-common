#include <Windows.h>

#include <intrin.h>

#include "../vulkan/km_vulkan_core.h"
#include "km_app.h"
#include "km_input.h"

bool running_ = false;
bool windowPropertiesChanged_ = false;
bool windowSizeChanged_ = false;
bool windowCursorVisible_ = !WINDOW_LOCK_CURSOR;
AppInput* input_ = nullptr;
WINDOWPLACEMENT windowPlacementPrev_ = { sizeof(windowPlacementPrev_) };

FixedArray<char, PATH_MAX_LENGTH> logFilePath_;

struct ThreadInfo
{
    AppWorkQueue* queue;
    HANDLE handle;
    uint32 index;
};

bool TryDoNextWorkEntry(AppWorkQueue* queue, uint32 threadIndex)
{
    const uint32 read = queue->read;
    if (read == queue->write) {
        return false;
    }

    const uint32 newRead = (read + 1) % C_ARRAY_LENGTH(queue->entries);
    const uint32 index = InterlockedCompareExchange((LONG volatile *)&queue->read, newRead, read);
    if (index == read) {
        AppWorkEntry entry = queue->entries[index];
        entry.callback(threadIndex, queue, entry.data);
        InterlockedIncrement((LONG volatile*)&queue->entriesComplete);
    }

    return true;
}

DWORD WINAPI WorkerThreadProc(_In_ LPVOID lpParameter)
{
    ThreadInfo* threadInfo = (ThreadInfo*)lpParameter;
    while (true) {
        if (!TryDoNextWorkEntry(threadInfo->queue, threadInfo->index)) {
            WaitForSingleObjectEx(threadInfo->queue->win32SemaphoreHandle, INFINITE, FALSE);
        }
    }

    return 0;
}

// TODO put in km_os
uint32 GetCpuCount()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return (uint32)systemInfo.dwNumberOfProcessors;
}

void CompleteAllWork(AppWorkQueue* queue, uint32 threadIndex)
{
    while (queue->entriesComplete != queue->entriesTotal) {
        TryDoNextWorkEntry(queue, threadIndex);
    }

    queue->entriesTotal = 0;
    queue->entriesComplete = 0;
}

bool TryAddWork(AppWorkQueue* queue, AppWorkQueueCallbackFunction* callback, void* data)
{
    const uint32 write = queue->write;
    const uint32 nextWrite = (write + 1) % C_ARRAY_LENGTH(queue->entries);
    if (nextWrite == queue->read) {
        return false;
    }

    AppWorkEntry* newEntry = &queue->entries[write];
    newEntry->callback = callback;
    newEntry->data = data;
    ++queue->entriesTotal;

    _WriteBarrier();

    queue->write = nextWrite;
    ReleaseSemaphore(queue->win32SemaphoreHandle, 1, NULL);

    return true;
}

bool IsCursorLocked()
{
    return !windowCursorVisible_;
}

void LockCursor(bool locked)
{
    // Such a weird api...
    const int targetDisplayCount = locked ? -1 : 0;

    int displayCount = ShowCursor(TRUE);
    while (displayCount != targetDisplayCount) {
        displayCount = ShowCursor(displayCount < targetDisplayCount);
    }

    windowCursorVisible_ = !locked;
}

void LogString(const char* str, uint32 n)
{
    const int LOG_STRING_BUFFER_SIZE = 1024;
    char buffer[LOG_STRING_BUFFER_SIZE];

    uint32 remaining = n;
    while (remaining > 0) {
        uint32 copySize = remaining;
        if (copySize > LOG_STRING_BUFFER_SIZE - 1) {
            copySize = LOG_STRING_BUFFER_SIZE - 1;
        }
        MemCopy(buffer, str, copySize);
        buffer[copySize] = '\0';
        OutputDebugString(buffer);
        printf("%s", buffer);
        remaining -= copySize;
    }

    Array<uint8> logData = { .size = n, .data = (uint8*)str };
    if (!WriteFile(logFilePath_.ToArray(), logData, true)) {
        DEBUG_PANIC("failed to write to log file");
    }
}

void PlatformFlushLogs(LogState* logState)
{
    // TODO fix this
    for (uint32 i = 0; i < logState->eventCount; i++) {
        uint32 eventIndex = (logState->eventFirst + i) % logState->logEvents.SIZE;
        const LogEvent& event = logState->logEvents[eventIndex];
        uint32 bufferStart = event.logStart;
        uint32 bufferEnd = event.logStart + event.logSize;
        if (bufferEnd >= logState->buffer.SIZE) {
            bufferEnd -= logState->buffer.SIZE;
        }
        if (bufferEnd >= bufferStart) {
            LogString(logState->buffer.data + bufferStart, event.logSize);
        }
        else {
            LogString(logState->buffer.data + bufferStart, logState->buffer.SIZE - bufferStart);
            LogString(logState->buffer.data, bufferEnd);
        }
    }

    if (logState->eventCount > 0) {
        fflush(stdout);
    }

    logState->eventFirst = (logState->eventFirst + logState->eventCount) % logState->logEvents.SIZE;
    logState->eventCount = 0;
    // uint64 toRead1, toRead2;
    // if (logState->readIndex <= logState->writeIndex) {
    //  toRead1 = logState->writeIndex - logState->readIndex;
    //  toRead2 = 0;
    // }
    // else {
    //  toRead1 = LOG_BUFFER_SIZE - logState->readIndex;
    //  toRead2 = logState->writeIndex;
    // }
    // if (toRead1 != 0) {
    //  LogString(logState->buffer + logState->readIndex, toRead1);
    // }
    // if (toRead2 != 0) {
    //  LogString(logState->buffer, toRead2);
    // }
    // logState->readIndex += toRead1 + toRead2;
    // if (logState->readIndex >= LOG_BUFFER_SIZE) {
    //  logState->readIndex -= LOG_BUFFER_SIZE;
    // }
}

KmKeyCode Win32KeyCodeToKm(int vkCode)
{
    // Numbers, letters, text
    if (vkCode >= 0x30 && vkCode <= 0x39) {
        return (KmKeyCode)(vkCode - 0x30 + KM_KEY_0);
    }
    else if (vkCode >= 0x41 && vkCode <= 0x5a) {
        return (KmKeyCode)(vkCode - 0x41 + KM_KEY_A);
    }
    else if (vkCode == VK_SPACE) {
        return KM_KEY_SPACE;
    }
    else if (vkCode == VK_BACK) {
        return KM_KEY_BACKSPACE;
    }
    // Arrow keys
    else if (vkCode == VK_UP) {
        return KM_KEY_ARROW_UP;
    }
    else if (vkCode == VK_DOWN) {
        return KM_KEY_ARROW_DOWN;
    }
    else if (vkCode == VK_LEFT) {
        return KM_KEY_ARROW_LEFT;
    }
    else if (vkCode == VK_RIGHT) {
        return KM_KEY_ARROW_RIGHT;
    }
    // Special keys
    else if (vkCode == VK_ESCAPE) {
        return KM_KEY_ESCAPE;
    }
    else if (vkCode == VK_SHIFT) {
        return KM_KEY_SHIFT;
    }
    else if (vkCode == VK_CONTROL) {
        return KM_KEY_CTRL;
    }
    else if (vkCode == VK_TAB) {
        return KM_KEY_TAB;
    }
    else if (vkCode == VK_RETURN) {
        return KM_KEY_ENTER;
    }
    else if (vkCode >= 0x60 && vkCode <= 0x69) {
        return (KmKeyCode)(vkCode - 0x60 + KM_KEY_NUMPAD_0);
    }
    else {
        return KM_KEY_INVALID;
    }
}

Vec2Int Win32GetRenderingViewportSize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    return Vec2Int { clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };
}

void Win32ToggleFullscreen(HWND hWnd)
{
    // This follows Raymond Chen's perscription for fullscreen toggling. See:
    // https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353

    DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        // Switch to fullscreen
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
        if (GetWindowPlacement(hWnd, &windowPlacementPrev_) && GetMonitorInfo(hMonitor, &monitorInfo)) {
            SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hWnd, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        // Switch to windowed
        SetWindowLong(hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hWnd, &windowPlacementPrev_);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {
        case WM_ACTIVATEAPP: {
            if (WINDOW_LOCK_CURSOR) {
                if (wParam == FALSE && !windowCursorVisible_) {
                    LockCursor(false);
                }
                else if (wParam == TRUE && windowCursorVisible_) {
                    LockCursor(true);
                }
            }
        } break;
        case WM_CLOSE: {
            // TODO handle this with a message?
            running_ = false;
        } break;
        case WM_DESTROY: {
            // TODO handle this as an error?
            running_ = false;
        } break;

        case WM_SIZE: {
            // TODO this is triggering once on app startup.
            // should I check if size actually changed before actioning on this?
            windowSizeChanged_ = true;
        } break;

        case WM_SYSKEYDOWN: {
            DEBUG_PANIC("WM_SYSKEYDOWN in WndProc");
        } break;
        case WM_SYSKEYUP: {
            DEBUG_PANIC("WM_SYSKEYUP in WndProc");
        } break;
        case WM_KEYDOWN: {
        } break;
        case WM_KEYUP: {
        } break;

        case WM_CHAR: {
            char c = (char)wParam;
            if (c == '\r') c = '\n';
            input_->keyboardString[input_->keyboardStringLen++] = c;
            input_->keyboardString[input_->keyboardStringLen] = '\0';
        } break;

        default: {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        } break;
    }

    return result;
}

internal void Win32ProcessMessages(HWND hWnd, AppInput* input)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_QUIT: {
                running_ = false;
            } break;

            case WM_SYSKEYDOWN: {
                uint32 vkCode = (uint32)msg.wParam;
                bool altDown = (msg.lParam & (1 << 29)) != 0;

                if (vkCode == VK_F4 && altDown) {
                    running_ = false;
                }
            } break;
            case WM_SYSKEYUP: {
            } break;

            case WM_KEYDOWN: {
                uint32 vkCode = (uint32)msg.wParam;
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                bool isDown = ((msg.lParam & (1 << 31)) == 0);
                int transitions = (wasDown != isDown) ? 1 : 0;
                DEBUG_ASSERT(isDown);

                int kmKeyCode = Win32KeyCodeToKm(vkCode);
                if (kmKeyCode != KM_KEY_INVALID) {
                    input->keyboard[kmKeyCode].isDown = isDown;
                    input->keyboard[kmKeyCode].transitions = transitions;
                }

                if (vkCode == VK_F11) {
                    Win32ToggleFullscreen(hWnd);
                    windowPropertiesChanged_ = true;
                    windowSizeChanged_ = true;
                }

                // Pass over to WndProc for WM_CHAR messages (string input)
                input_ = input;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
            case WM_KEYUP: {
                uint32 vkCode = (uint32)msg.wParam;
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                bool isDown = ((msg.lParam & (1 << 31)) == 0);
                int transitions = (wasDown != isDown) ? 1 : 0;
                DEBUG_ASSERT(!isDown);

                int kmKeyCode = Win32KeyCodeToKm(vkCode);
                if (kmKeyCode != KM_KEY_INVALID) {
                    input->keyboard[kmKeyCode].isDown = isDown;
                    input->keyboard[kmKeyCode].transitions = transitions;
                }
            } break;

            case WM_LBUTTONDOWN: {
                input->mouseButtons[KM_MOUSE_LEFT].isDown = true;
                input->mouseButtons[KM_MOUSE_LEFT].transitions = 1;
            } break;
            case WM_LBUTTONUP: {
                input->mouseButtons[KM_MOUSE_LEFT].isDown = false;
                input->mouseButtons[KM_MOUSE_LEFT].transitions = 1;
            } break;
            case WM_RBUTTONDOWN: {
                input->mouseButtons[KM_MOUSE_RIGHT].isDown = true;
                input->mouseButtons[KM_MOUSE_RIGHT].transitions = 1;
            } break;
            case WM_RBUTTONUP: {
                input->mouseButtons[KM_MOUSE_RIGHT].isDown = false;
                input->mouseButtons[KM_MOUSE_RIGHT].transitions = 1;
            } break;
            case WM_MBUTTONDOWN: {
                input->mouseButtons[KM_MOUSE_MIDDLE].isDown = true;
                input->mouseButtons[KM_MOUSE_MIDDLE].transitions = 1;
            } break;
            case WM_MBUTTONUP: {
                input->mouseButtons[KM_MOUSE_MIDDLE].isDown = false;
                input->mouseButtons[KM_MOUSE_MIDDLE].transitions = 1;
            } break;
            case WM_XBUTTONDOWN: {
                if ((msg.wParam & MK_XBUTTON1) != 0) {
                    input->mouseButtons[KM_MOUSE_ALT1].isDown = true;
                    input->mouseButtons[KM_MOUSE_ALT1].transitions = 1;
                }
                else if ((msg.wParam & MK_XBUTTON2) != 0) {
                    input->mouseButtons[KM_MOUSE_ALT2].isDown = true;
                    input->mouseButtons[KM_MOUSE_ALT2].transitions = 1;
                }
            } break;
            case WM_XBUTTONUP: {
                if ((msg.wParam & MK_XBUTTON1) != 0) {
                    input->mouseButtons[KM_MOUSE_ALT1].isDown = false;
                    input->mouseButtons[KM_MOUSE_ALT1].transitions = 1;
                }
                else if ((msg.wParam & MK_XBUTTON2) != 0) {
                    input->mouseButtons[KM_MOUSE_ALT2].isDown = false;
                    input->mouseButtons[KM_MOUSE_ALT2].transitions = 1;
                }
            } break;
            case WM_MOUSEWHEEL: {
                // TODO standardize these units
                input->mouseWheel += GET_WHEEL_DELTA_WPARAM(msg.wParam);
            } break;

            default: {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

HWND Win32CreateWindow(HINSTANCE hInstance, WNDPROC wndProc, const char* className, const char* windowName,
                       int x, int y, int clientWidth, int clientHeight)
{
    WNDCLASSEX wndClass = { sizeof(wndClass) };
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = wndProc;
    wndClass.hInstance = hInstance;
    //wndClass.hIcon = NULL;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.lpszClassName = className;

    if (!RegisterClassEx(&wndClass)) {
        LOG_ERROR("RegisterClassEx call failed\n");
        return NULL;
    }

    RECT windowRect   = {};
    windowRect.left   = x;
    windowRect.top    = y;
    windowRect.right  = x + clientWidth;
    windowRect.bottom = y + clientHeight;

    if (!AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE, 0)) {
        LOG_ERROR("AdjustWindowRectEx call failed\n");
        GetLastError();
        return NULL;
    }

    HWND hWindow = CreateWindowEx(0, className, windowName,
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  windowRect.left, windowRect.top,
                                  windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                                  0, 0, hInstance, 0);

    if (!hWindow) {
        LOG_ERROR("CreateWindowEx call failed\n");
        return NULL;
    }

    return hWindow;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    int n = stbsp_snprintf(logFilePath_.data, PATH_MAX_LENGTH,
                           "logs/log%04d-%02d-%02d_%02d-%02d-%02d.txt",
                           systemTime.wYear, systemTime.wMonth, systemTime.wDay,
                           systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
    logFilePath_.size += n;

    LogState* logState = (LogState*)VirtualAlloc(0, sizeof(LogState), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!logState) {
        LOG_ERROR("Log state memory allocation failed\n");
        PlatformFlushLogs(logState);
        return 1;
    }
    logState->eventFirst = 0;
    logState->eventCount = 0;
    logState_ = logState;

    HWND hWnd = Win32CreateWindow(hInstance, WndProc, "VulkanWindowClass", WINDOW_NAME,
                                  100, 100, WINDOW_START_WIDTH, WINDOW_START_HEIGHT);
    if (!hWnd) {
        LOG_ERROR("Win32CreateWindow failed\n");
        LOG_FLUSH();
        return 1;
    }

#if GAME_INTERNAL
    LPVOID baseAddress = (LPVOID)TERABYTES(2);
#else
    LPVOID baseAddress = 0;
#endif

    // Initialize app memory
    LargeArray<uint8> totalMemory;
    totalMemory.size = PERMANENT_MEMORY_SIZE + TRANSIENT_MEMORY_SIZE;
    totalMemory.data = (uint8*)VirtualAlloc(baseAddress, totalMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!totalMemory.data) {
        LOG_ERROR("Win32 memory allocation failed\n");
        LOG_FLUSH();
        return 1;
    }
    AppMemory appMemory = {
        .initialized = false,
        .permanent = {
            .size = PERMANENT_MEMORY_SIZE,
            .data = totalMemory.data
        },
        .transient = {
            .size = TRANSIENT_MEMORY_SIZE,
            .data = totalMemory.data + PERMANENT_MEMORY_SIZE
        }
    };
    LOG_INFO("Initialized app memory, %llu bytes\n", totalMemory.size);

    // Initialize Vulkan
    Vec2Int windowSize = { WINDOW_START_WIDTH, WINDOW_START_HEIGHT };
    VulkanState vulkanState;
    {
        LinearAllocator tempAllocator(appMemory.transient);
        if (!LoadVulkanState(&vulkanState, hInstance, hWnd, windowSize, &tempAllocator)) {
            LOG_ERROR("LoadVulkanState failed\n");
            LOG_FLUSH();
            return 1;
        }
    }
    LOG_INFO("Loaded Vulkan state, %lu swapchain images\n", vulkanState.swapchain.images.size);

    if (!AppLoadVulkanWindowState(vulkanState, &appMemory)) {
        LOG_ERROR("AppLoadVulkanWindowState failed\n");
        LOG_FLUSH();
        return 1;
    }
    if (!AppLoadVulkanSwapchainState(vulkanState, &appMemory)) {
        LOG_ERROR("AppLoadVulkanSwapchainState failed\n");
        LOG_FLUSH();
        return 1;
    }

    // Initialize audio
    AppAudio appAudio = {};

    // Initialize input
    AppInput input[2] = {};
    AppInput *newInput = &input[0];
    AppInput *oldInput = &input[1];
    if (WINDOW_LOCK_CURSOR) {
        LockCursor(true);
    }

    // Initialize app work queue
    AppWorkQueue appWorkQueue;
    const int MAX_THREADS = 256;
    FixedArray<ThreadInfo, MAX_THREADS> threads;
    threads.Clear();
    {
        appWorkQueue.entriesTotal = 0;
        appWorkQueue.entriesComplete = 0;
        appWorkQueue.read = 0;
        appWorkQueue.write = 0;
        appWorkQueue.win32SemaphoreHandle = CreateSemaphoreEx(NULL, 0, C_ARRAY_LENGTH(appWorkQueue.entries),
                                                              NULL, 0, SEMAPHORE_ALL_ACCESS);
        if (appWorkQueue.win32SemaphoreHandle == NULL) {
            LOG_ERROR("Failed to create AppWorkQueue semaphore\n");
            LOG_FLUSH();
            return 1;
        }

        uint32 numThreads = GetCpuCount() - 1;
        if (numThreads > MAX_THREADS) {
            LOG_INFO("hello future! Machine has too many processors: %d, clamping to %d\n", numThreads, MAX_THREADS);
            numThreads = MAX_THREADS;
        }
#if !ENABLE_THREADS
        numThreads = 0;
#endif

        for (uint32 i = 0; i < numThreads; i++) {
            ThreadInfo* threadInfo = threads.Append();
            threadInfo->queue = &appWorkQueue;
            threadInfo->index = i + 1;

            threadInfo->handle = CreateThread(NULL, 0, WorkerThreadProc, threadInfo, 0, NULL);
            if (threadInfo->handle == NULL) {
                LOG_ERROR("Failed to create worker thread\n");
                LOG_FLUSH();
                return 1;
            }
        }
        LOG_INFO("Loaded work queue, %d threads\n", numThreads);
    }

    // Initialize timing information
    int64 timerFreq;
    LARGE_INTEGER timerLast;
    uint64 cyclesLast;
    {
        LARGE_INTEGER timerFreqResult;
        QueryPerformanceFrequency(&timerFreqResult);
        timerFreq = timerFreqResult.QuadPart;

        QueryPerformanceCounter(&timerLast);
        cyclesLast = __rdtsc();
    }
    float32 lastElapsed = 0.0f;

    running_ = true;
    while (running_) {
        int mouseWheelPrev = newInput->mouseWheel;
        Win32ProcessMessages(hWnd, newInput);
        newInput->mouseWheelDelta = newInput->mouseWheel - mouseWheelPrev;

        if (windowPropertiesChanged_ || windowSizeChanged_) {
            // TODO duplicate from vkAcquireNextImageKHR out of date case
            Vec2Int newSize = Win32GetRenderingViewportSize(hWnd);
            LinearAllocator tempAllocator(appMemory.transient);

            vkDeviceWaitIdle(vulkanState.window.device);
            AppUnloadVulkanSwapchainState(vulkanState, &appMemory);

            if (windowPropertiesChanged_) {
                AppUnloadVulkanWindowState(vulkanState, &appMemory);
                if (!ReloadVulkanWindow(&vulkanState, hInstance, hWnd, newSize, &tempAllocator)) {
                    DEBUG_PANIC("Failed to reload Vulkan window\n");
                }
                if (!AppLoadVulkanWindowState(vulkanState, &appMemory)) {
                    DEBUG_PANIC("Failed to reload Vulkan window-dependent app state\n");
                }
            }
            else {
                if (!ReloadVulkanSwapchain(&vulkanState, newSize, &tempAllocator)) {
                    DEBUG_PANIC("Failed to reload Vulkan swapchain\n");
                }
            }

            if (!AppLoadVulkanSwapchainState(vulkanState, &appMemory)) {
                DEBUG_PANIC("Failed to reload Vulkan swapchain-dependent app state\n");
            }

            windowPropertiesChanged_ = false;
            windowSizeChanged_ = false;
            continue;
        }

        const Vec2Int screenSize = {
            (int)vulkanState.swapchain.extent.width,
            (int)vulkanState.swapchain.extent.height
        };

        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(hWnd, &mousePos);

        Vec2Int mousePosPrev = newInput->mousePos;
        if (WINDOW_LOCK_CURSOR && !windowCursorVisible_) {
            const Vec2Int screenMid = screenSize / 2;

            POINT windowMidScreen;
            windowMidScreen.x = screenMid.x;
            windowMidScreen.y = screenMid.y;
            ClientToScreen(hWnd, &windowMidScreen);

            SetCursorPos(windowMidScreen.x, windowMidScreen.y);
            mousePosPrev = screenMid;
        }

        newInput->mousePos.x = mousePos.x;
        newInput->mousePos.y = mousePos.y;
        newInput->mouseDelta = newInput->mousePos - mousePosPrev;
        if (mousePos.x < 0 || mousePos.x > screenSize.x || mousePos.y < 0 || mousePos.y > screenSize.y) {
            for (int i = 0; i < 5; i++) {
                int transitions = newInput->mouseButtons[i].isDown ? 1 : 0;
                newInput->mouseButtons[i].isDown = false;
                newInput->mouseButtons[i].transitions = transitions;
            }
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vulkanState.window.device, vulkanState.swapchain.swapchain,
                                                UINT64_MAX, vulkanState.window.imageAvailableSemaphore, VK_NULL_HANDLE,
                                                &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // TODO duplicate from windowSizeChange_
            Vec2Int newSize = Win32GetRenderingViewportSize(hWnd);
            LinearAllocator tempAllocator(appMemory.transient);

            vkDeviceWaitIdle(vulkanState.window.device);
            AppUnloadVulkanSwapchainState(vulkanState, &appMemory);
            if (!ReloadVulkanWindow(&vulkanState, hInstance, hWnd, newSize, &tempAllocator)) {
                DEBUG_PANIC("Failed to reload Vulkan window\n");
            }
            if (!AppLoadVulkanSwapchainState(vulkanState, &appMemory)) {
                DEBUG_PANIC("Failed to reload Vulkan swapchain-dependent app state\n");
            }
            continue;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            LOG_ERROR("Failed to acquire swapchain image\n");
        }

        bool shouldRender = AppUpdateAndRender(vulkanState, imageIndex, *newInput, lastElapsed,
                                               &appMemory, &appAudio, &appWorkQueue);
        if (!shouldRender) {
            // TODO lmao, plz fix... how do we cancel the frame cleanly?
            continue;
        }

        // TODO not sure what the split in Vulkan rendering responsibility should be between
        // the OS and game layers...
        const VkSemaphore signalSemaphores[] = { vulkanState.window.renderFinishedSemaphore };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = C_ARRAY_LENGTH(signalSemaphores);
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vulkanState.swapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(vulkanState.window.presentQueue, &presentInfo);

        // timing information
        {
            LARGE_INTEGER timerEnd;
            QueryPerformanceCounter(&timerEnd);
            uint64 cyclesEnd = __rdtsc();

            int64 timerElapsed = timerEnd.QuadPart - timerLast.QuadPart;
            lastElapsed = (float32)timerElapsed / timerFreq;
            // float32 elapsedMs = lastElapsed * 1000.0f;
            // int64 cyclesElapsed = cyclesEnd - cyclesLast;
            // float64 megaCyclesElapsed = (float64)cyclesElapsed / 1000000.0f;
            // LOG_INFO("elapsed %.03f ms | %.03f MC\n", elapsedMs, megaCyclesElapsed);

            timerLast = timerEnd;
            cyclesLast = cyclesEnd;
        }

        AppInput *temp = newInput;
        newInput = oldInput;
        oldInput = temp;
        ClearInput(newInput, *oldInput);

        LOG_FLUSH();
    }

    vkDeviceWaitIdle(vulkanState.window.device);
    AppUnloadVulkanSwapchainState(vulkanState, &appMemory);
    AppUnloadVulkanWindowState(vulkanState, &appMemory);
    UnloadVulkanState(&vulkanState);

    return 0;
}
