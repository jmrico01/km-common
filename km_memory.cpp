#include "km_memory.h"

#include <stdlib.h>

void* StandardAllocator::Allocate(uint64 size)
{
	return malloc(size);
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
	: used(0), capacity(capacity), lastAllocatedSize(0), data(data)
{
}

void* LinearAllocator::Allocate(uint64 size)
{
	if ((capacity - used) < size) {
		return nullptr;
	}

	lastAllocatedSize = size;
	uint64 start = used;
	used += size;
	return (void*)((uint8*)data + start);
}

void* LinearAllocator::ReAllocate(void* memory, uint64 size)
{
	return Allocate(size); // TODO lmao so lazy
}

void LinearAllocator::Free(void* memory)
{
	uint64 memoryInd = (uint64)memory - (uint64)data;
	DEBUG_ASSERT(memoryInd < capacity);
	DEBUG_ASSERT(memoryInd + lastAllocatedSize == used);
	used = memoryInd;
	lastAllocatedSize = 0; // Can't validate more calls to Free after this
}

LinearAllocatorState LinearAllocator::SaveState()
{
	LinearAllocatorState state;
	state.used = used;
	state.lastAllocatedSize = lastAllocatedSize;
	return state;
}

void LinearAllocator::LoadState(const LinearAllocatorState& state)
{
	used = state.used;
	lastAllocatedSize = state.lastAllocatedSize;
}