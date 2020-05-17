#pragma once

#include "km_lib.h"
#include "km_memory.h"
#include "km_string.h"

enum class KmkvItemType
{
    NONE,
    STRING,
    KMKV
};

template <typename Allocator = StandardAllocator>
struct KmkvItem
{
    DynamicArray<char, Allocator> keywordTag;

    KmkvItemType type;
    DynamicArray<char, Allocator>* dynamicStringPtr;
    HashTable<KmkvItem<Allocator>, Allocator>* hashTablePtr;

    KmkvItem();
    KmkvItem(const KmkvItem<Allocator>& other) = delete;
    KmkvItem<Allocator>& operator=(const KmkvItem<Allocator>& other);
    ~KmkvItem();
};

int ReadNextKeywordValue(const_string str, string* outKeyword, string* outValue);

// NOTE(patio) deprecated! just use the way simpler API
template <uint64 KEYWORD_SIZE, typename Allocator>
int ReadNextKeywordValue(const Array<char>& str,
                         FixedArray<char, KEYWORD_SIZE>* outKeyword, DynamicArray<char, Allocator>* outValue);

template <typename Allocator>
DynamicArray<char, Allocator>* GetKmkvItemStrValue(HashTable<KmkvItem<Allocator>, Allocator>& kmkv,
                                                   const HashKey& itemKey);
template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemStrValue(const HashTable<KmkvItem<Allocator>, Allocator>& kmkv,
                                                         const HashKey& itemKey);
template <typename Allocator>
HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(HashTable<KmkvItem<Allocator>, Allocator>& kmkv,
                                                    const HashKey& itemKey);
template <typename Allocator>
const HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(const HashTable<KmkvItem<Allocator>, Allocator>& kmkv,
                                                          const HashKey& itemKey);

template <typename Allocator>
bool LoadKmkv(const Array<char>& filePath, Allocator* allocator,
              HashTable<KmkvItem<Allocator>>* outKmkv);

template <typename Allocator>
bool LoadKmkv(const Array<char>& kmkvString, HashTable<KmkvItem<Allocator>, Allocator>* outKmkv);

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
