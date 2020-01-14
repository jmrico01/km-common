#pragma once

#include "km_lib.h"
#include "km_memory.h"

template <typename Allocator = StandardAllocator>
struct KmkvItem
{
	DynamicArray<char, Allocator> keywordTag;

	bool isString;
	DynamicArray<char, Allocator>* dynamicStringPtr;
	HashTable<KmkvItem<Allocator>>* hashTablePtr;
};


template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemStrValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey);
template <typename Allocator>
const HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey);

template <uint64 KEYWORD_SIZE, uint64 VALUE_SIZE>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, FixedArray<char, VALUE_SIZE>* outValue);

template <typename Allocator>
internal bool LoadKmkv(const Array<char>& filePath, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outHashTable);