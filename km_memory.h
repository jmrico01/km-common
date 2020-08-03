#pragma once

#include "km_array.h"
#include "km_defines.h"

// Pushes allocator state onto the stack
// Resets the state when the current scope ends using a defer statement
#define SCOPED_ALLOCATOR_RESET(allocator) auto KM_UNIQUE_NAME_LINE(scopedState) = (allocator).SaveState(); \
defer((allocator).LoadState(KM_UNIQUE_NAME_LINE(scopedState)));

void MemCopy(void* dst, const void* src, uint64 numBytes);
void MemMove(void* dst, const void* src, uint64 numBytes);
void MemSet(void* dst, uint8 value, uint64 numBytes);
int  MemComp(const void* mem1, const void* mem2, uint64 numBytes);

struct StandardAllocator
{
    void* Allocate(uint64 size);
    template <typename T> T* New();
    void* ReAllocate(void* memory, uint64 size);
    void Free(void* memory);
};

StandardAllocator defaultAllocator_;

struct LinearAllocatorState
{
    uint64 used;
};

struct LinearAllocator
{
    uint64 used;
    uint64 capacity;
    void* data;

    LinearAllocator(const LargeArray<uint8>& memory);
    LinearAllocator(uint64 capacity, void* data);

    void Initialize(const LargeArray<uint8>& memory);
    void Initialize(uint64 capacity, void* data);
    void Clear();
    uint64 GetRemainingBytes();

    void* Allocate(uint64 size);
    template <typename T> T* New();
    template <typename T> T* New(uint64 size);
    template <typename T> Array<T> NewArray(uint32 size);
    void* ReAllocate(void* memory, uint64 size);
    void Free(void* memory);

    LinearAllocatorState SaveState();
    void LoadState(const LinearAllocatorState& state);
};
