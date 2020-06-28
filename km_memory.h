#pragma once

#include "km_array.h"
#include "km_defines.h"

void MemCopy(void* dst, const void* src, uint64 numBytes);
void MemMove(void* dst, const void* src, uint64 numBytes);
void MemSet(void* dst, char value, uint64 numBytes);
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

    LinearAllocator(const Array<uint8>& memory);
    LinearAllocator(uint64 capacity, void* data);

    void* Allocate(uint64 size);
    template <typename T> T* New();
    void* ReAllocate(void* memory, uint64 size);
    void Free(void* memory);
    void Clear();

    LinearAllocatorState SaveState();
    void LoadState(const LinearAllocatorState& state);
};
