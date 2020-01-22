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

template <uint64 KEYWORD_SIZE, typename Allocator>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, DynamicArray<char, Allocator>* outValue);

template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemStrValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey);
template <typename Allocator>
const HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey);

template <typename Allocator>
bool LoadKmkv(const Array<char>& filePath, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv);
template <typename Allocator>
void FreeKmkv(const HashTable<KmkvItem<Allocator>>& kmkv);

template <typename Allocator>
bool KmkvToString(const HashTable<KmkvItem<Allocator>>& kmkv,
	DynamicArray<char, Allocator>* outString);

#ifdef KM_KMKV_JSON
template <typename Allocator>
bool KmkvToJson(const HashTable<KmkvItem<Allocator>>& kmkv, DynamicArray<char, Allocator>* outJson);
template <typename Allocator>
bool JsonToKmkv(const Array<char>& jsonString, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv);
#endif
