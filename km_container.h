#pragma once

#include "km_array.h"
#include "km_defines.h"
#include "km_memory.h"

// NOTE: Adding things to this container might invalidate pointers to elements.
// Subtle case that confused me: getting pointers through Append 3 times in a row, and only afterward
// setting the 3 values through the pointers. Some values would be unset if a resize was triggered.
template <typename T, typename Allocator = StandardAllocator>
struct DynamicArray
{
    uint64 size;
    T* data;
    uint64 capacity;
    Allocator* allocator;

    DynamicArray();
    DynamicArray(Allocator* allocator);
    DynamicArray(const Array<T>& array, Allocator* allocator = nullptr);
    DynamicArray(uint64 capacity, Allocator* allocator = nullptr);
    DynamicArray(const DynamicArray<T>& other) = delete;

    Array<T> ToArray() const;
    void FromArray(const Array<T>& array);

    T* Append();
    T* Append(const T& element);
    void Append(const Array<T>& array);
    void Append(const Array<const T>& array);
    void RemoveLast();
    void Clear();
    uint64 IndexOf(const T& value);
    void Free();

    inline T& operator[](int index);
    inline T& operator[](uint64 index);
    inline const T& operator[](int index) const;
    inline const T& operator[](uint64 index) const;

    DynamicArray<T, Allocator>& operator=(const DynamicArray<T, Allocator>& other);

    bool UpdateCapacity(uint64 newCapacity);
};

struct HashKey
{
    static const uint64 MAX_LENGTH = 64;

    FixedArray<char, MAX_LENGTH> s;

    HashKey();
    HashKey(Array<char> str); // TODO move somewhere else to use const_string?
    HashKey(const Array<const char> str); // TODO move somewhere else to use const_string?
    HashKey(const char* str);

    bool WriteString(const Array<const char> str);
    bool WriteString(const char* str);
};

template <typename V>
struct KeyValuePair
{
    HashKey key;
    V value;
};

template <typename V, typename Allocator = StandardAllocator>
struct HashTable
{
    uint64 size;
    uint64 capacity;
    KeyValuePair<V>* pairs;
    Allocator* allocator;

    HashTable();
    HashTable(Allocator* allocator);
    HashTable(uint64 capacity, Allocator* allocator = nullptr);
    HashTable(const HashTable<V, Allocator>& other) = delete;
    ~HashTable();

    void Add(const HashKey& key, const V& value);
    V* Add(const HashKey& key);
    V* GetValue(const HashKey& key);
    const V* GetValue(const HashKey& key) const;
    bool Remove(const HashKey& key);

    void Clear();
    void Free();

    HashTable<V, Allocator>& operator=(const HashTable<V, Allocator>& other);

    private:
    KeyValuePair<V>* GetPair(const HashKey& key) const;
    KeyValuePair<V>* GetFreeSlot(const HashKey& key);
};

bool KeyCompare(const HashKey& key1, const HashKey& key2);
