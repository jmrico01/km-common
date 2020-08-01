#include "km_memory.h"

#include <stdlib.h>

void* StandardAllocator::Allocate(uint64 size)
{
    return malloc(size);
}

template <typename T> T* StandardAllocator::New()
{
    return (T*)Allocate(sizeof(T));
}

template <typename T> T* StandardAllocator::New(uint64 size)
{
    return (T*)Allocate(sizeof(T) * size);
}

void* StandardAllocator::ReAllocate(void* memory, uint64 size)
{
    return realloc(memory, size);
}

void StandardAllocator::Free(void* memory)
{
    free(memory);
}

LinearAllocator::LinearAllocator(uint64 capacity, void* data)
: used(0), capacity(capacity), data(data)
{
}

void* LinearAllocator::Allocate(uint64 size)
{
    if (used + size > capacity) {
        return nullptr;
    }

    uint64 start = used;
    used += size;
    return (void*)((uint8*)data + start);
}

template <typename T> T* LinearAllocator::New()
{
    return (T*)Allocate(sizeof(T));
}

void* LinearAllocator::ReAllocate(void* memory, uint64 size)
{
    void* newData = Allocate(size);
    // TODO extremely hacky way of copying memory, but otherwise, we would have had to know the previous alloc size
    uint64 diff = (uint64)newData - (uint64)memory;
    MemCopy(newData, memory, MinUInt64(diff, size));
    return newData;
}

void LinearAllocator::Free(void* memory)
{
    DEBUG_ASSERT(memory >= data);
    uint64 memoryInd = (uint64)memory - (uint64)data;
    DEBUG_ASSERT(memoryInd < capacity);
    used = memoryInd;
}

LinearAllocatorState LinearAllocator::SaveState()
{
    LinearAllocatorState state;
    state.used = used;
    return state;
}

void LinearAllocator::LoadState(const LinearAllocatorState& state)
{
    used = state.used;
}