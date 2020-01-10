#pragma once

#include "km_defines.h"

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

	LinearAllocator(uint64 capacity, void* data);

	void* Allocate(uint64 size);
	template <typename T> T* New();
	void* ReAllocate(void* memory, uint64 size);
	void Free(void* memory);
	void Clear();

	LinearAllocatorState SaveState();
	void LoadState(const LinearAllocatorState& state);
};