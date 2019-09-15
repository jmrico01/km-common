#pragma once

#include "km_defines.h"

struct LinearAllocatorState
{
	uint64 used;
	uint64 lastAllocatedSize;
};

struct LinearAllocator
{
	uint64 used;
	uint64 capacity;
	uint64 lastAllocatedSize;
	void* data;

	LinearAllocator(uint64 capacity, void* data);

	void* Allocate(uint64 size);
	void Free(void* memory);
	void Clear();

	LinearAllocatorState SaveState();
	void LoadState(const LinearAllocatorState& state);
};