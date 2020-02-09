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
	if ((capacity - used) < size) {
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
	return Allocate(size); // TODO lmao so lazy
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