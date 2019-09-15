#include "km_memory.h"

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

void LinearAllocator::Free(void* memory)
{
	uint64 memoryInd = (uint64)memory - (uint64)data;
	DEBUG_ASSERT(memoryInd < capacity);
	DEBUG_ASSERT(memoryInd + lastAllocatedSize == used);
	used = memoryInd;
	lastAllocatedSize = 0;
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