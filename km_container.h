#pragma once

#include "km_defines.h"
#include "km_lib.h"
#include "km_memory.h"

// TODO Assignment (=) of DynamicArray shallow-copies their members. Override the operator, probably
// OMG please don't do this ^   get rid of the whole RAII-ness of this thing ASAP, it's so annoying
template <typename T, typename Allocator = StandardAllocator>
struct DynamicArray // TODO figure out where allocator will go
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
    ~DynamicArray();

    Array<T>& ToArray() const; // TODO(patio): hmm, I don't like this
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
    FixedArray<char, HASHKEY_MAX_LENGTH> s;

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
