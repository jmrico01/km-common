#include "km_lib.h"

#include <cstring> // memcpy is here... no idea why
#include <limits.h>

#include "km_math.h"
#include "km_string.h"

#define DYNAMIC_ARRAY_START_CAPACITY 16
#define DYNAMIC_QUEUE_START_CAPACITY 16

// TODO pretty high, maybe do lower
#define HASH_TABLE_START_CAPACITY 89
#define HASH_TABLE_MAX_SIZE_TO_CAPACITY 0.7

internal inline void ArrayBoundsCheck(int index, uint64 size)
{
    DEBUG_ASSERTF(0 <= index && (uint64)index < size,
                  "Array bounds check failed: index %d, size %" PRIu64 "\n", index, size);
}

internal inline void ArrayBoundsCheck(uint64 index, uint64 size)
{
    DEBUG_ASSERTF(index < size,
                  "Array bounds check failed: index %" PRIu64 ", size %" PRIu64 "\n", index, size);
}

int ToIntOrTruncate(uint64 n)
{
    if (n > INT_MAX) {
        return INT_MAX;
    }
    else {
        return (int)n;
    }
}

inline uint32 SafeTruncateUInt64(uint64 value)
{
    DEBUG_ASSERT(value <= 0xFFFFFFFF);
    uint32 result = (uint32)value;
    return result;
}

void MemCopy(void* dst, const void* src, uint64 numBytes)
{
    DEBUG_ASSERT(((const char*)dst + numBytes <= src)
                 || (dst >= (const char*)src + numBytes));
    // TODO maybe see about reimplementing this? would be informative
    memcpy(dst, src, numBytes);
}

void MemMove(void* dst, const void* src, uint64 numBytes)
{
    memmove(dst, src, numBytes);
}

void MemSet(void* dst, char value, uint64 numBytes)
{
    memset(dst, value, numBytes);
}

int MemComp(const void* mem1, const void* mem2, uint64 numBytes)
{
    return memcmp(mem1, mem2, numBytes);
}

// Very simple string hash ( djb2 hash, source http://www.cse.yorku.ca/~oz/hash.html )
uint64 KeyHash(const HashKey& key)
{
    uint64 hash = 5381;

    for (uint64 i = 0; i < key.s.size; i++) {
        hash = ((hash << 5) + hash) + key.s[i];
    }

    return hash;
}
bool KeyCompare(const HashKey& key1, const HashKey& key2)
{
    if (key1.s.size != key2.s.size) {
        return false;
    }

    for (uint64 i = 0; i < key1.s.size; i++) {
        if (key1.s[i] != key2.s[i]) {
            return false;
        }
    }

    return true;
}

template <typename T> const Array<T> Array<T>::empty = { .size = 0, .data = nullptr };

template <typename T>
void Array<T>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T>
void Array<T>::Clear()
{
    size = 0;
}

template <typename T>
uint64 Array<T>::FindFirst(const T& value, uint64 start) const
{
    for (uint64 i = start; i < size; i++) {
        if (data[i] == value) {
            return i;
        }
    }
    return size;
}

template <typename T>
uint64 Array<T>::FindLast(const T& value) const
{
    for (uint64 i = size; i != 0; i--) {
        if (data[i - 1] == value) {
            return i - 1;
        }
    }
    return size;
}

template <typename T>
Array<T> Array<T>::Slice(uint64 start, uint64 end) const
{
    DEBUG_ASSERT(start < size);
    DEBUG_ASSERT(end <= size);
    DEBUG_ASSERT(start <= end);
    Array<T> slice;
    slice.data = &data[start];
    slice.size = end - start;
    return slice;
}

template <typename T>
Array<T> Array<T>::SliceTo(uint64 end) const
{
    return Slice(0, end);
}

template <typename T>
Array<T> Array<T>::SliceFrom(uint64 start) const
{
    return Slice(start, size);
}

template <typename T>
inline T& Array<T>::operator[](int index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T>
inline T& Array<T>::operator[](uint64 index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](int index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint64 index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, uint64 S>
Array<T> FixedArray<T, S>::ToArray() const
{
    return {
        .size = size,
        .data = (T*)data
    };
}

template <typename T, uint64 S>
const Array<const T> FixedArray<T, S>::ToConstArray() const
{
    return {
        .size = size,
        .data = (T*)data
    };
}

template <typename T, uint64 S>
void FixedArray<T, S>::FromArray(const Array<T>& array)
{
    DEBUG_ASSERT(array.size <= S);
    MemCopy(data, array.data, array.size * sizeof(T));
    size = array.size;
}

template <typename T, uint64 S>
T* FixedArray<T, S>::Append()
{
    DEBUG_ASSERTF(size < S, "size %" PRIu64 ", S %" PRIu64 "\n", size, S);
    return &data[size++];
}

template <typename T, uint64 S>
T* FixedArray<T, S>::Append(const T& element)
{
    T* slot = Append();
    *slot = element;
    return slot;
}

template <typename T, uint64 S>
void FixedArray<T, S>::Append(const Array<T>& array)
{
    Append((const Array<const T>)array);
}

template <typename T, uint64 S>
void FixedArray<T, S>::Append(const Array<const T>& array)
{
    uint64 newSize = size + array.size;
    DEBUG_ASSERTF(newSize <= S, "size %" PRIu64 ", S %" PRIu64 ", array.size %" PRIu64 "\n",
                  size, S, array.size);

    for (uint64 i = 0; i < array.size; i++) {
        data[size + i] = array.data[i];
    }
    size = newSize;
}

template <typename T, uint64 S>
void FixedArray<T, S>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T, uint64 S>
void FixedArray<T, S>::Clear()
{
    size = 0;
}

template <typename T, uint64 S>
uint64 FixedArray<T, S>::IndexOf(const T& value)
{
    for (uint64 i = 0; i < size; i++) {
        if (data[i] == value) {
            return i;
        }
    }
    return size;
}

template <typename T, uint64 S>
void FixedArray<T, S>::AppendAfter(const T& element, uint64 index)
{
    DEBUG_ASSERT(index < size);
    DEBUG_ASSERTF(size < S, "size %" PRIu64 ", S %" PRIu64 "\n", size, S);

    uint64 targetIndex = index + 1;
    for (uint64 i = size; i > targetIndex; i--) {
        data[i] = data[i - 1];
    }
    data[targetIndex] = element;
    size++;
}

template <typename T, uint64 S>
void FixedArray<T, S>::Remove(uint64 index)
{
    DEBUG_ASSERT(size > 0);
    DEBUG_ASSERT(index < size);
    for (uint64 i = index + 1; i < size; i++) {
        data[i - 1] = data[i];
    }
    size--;
}

template <typename T, uint64 S>
inline T& FixedArray<T, S>::operator[](int index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline T& FixedArray<T, S>::operator[](uint64 index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline const T& FixedArray<T, S>::operator[](int index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline const T& FixedArray<T, S>::operator[](uint64 index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, uint64 S>
FixedArray<T, S>& FixedArray<T, S>::operator=(const FixedArray<T, S>& other)
{
    DEBUG_ASSERT(other.size <= S);
    size = other.size;
    MemCopy(data, other.data, size * sizeof(T));
    return *this;
}

// TODO dumb wrappers until I figure out a better way to do this at compile time
template <typename Allocator>
void* AllocateOrUseDefaultIfNull(Allocator* allocator, uint64 size)
{
    if (allocator == nullptr) {
        return defaultAllocator_.Allocate(size);
    }
    else {
        return allocator->Allocate(size);
    }
}
template <typename Allocator>
void* ReAllocateOrUseDefaultIfNull(Allocator* allocator, void* memory, uint64 size)
{
    if (allocator == nullptr) {
        return defaultAllocator_.ReAllocate(memory, size);
    }
    else {
        return allocator->ReAllocate(memory, size);
    }
}
template <typename Allocator>
void FreeOrUseDefautIfNull(Allocator* allocator, void* memory)
{
    if (allocator == nullptr) {
        defaultAllocator_.Free(memory);
    }
    else {
        allocator->Free(memory);
    }
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray()
: DynamicArray(nullptr)
{
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray(Allocator* allocator)
: DynamicArray(DYNAMIC_ARRAY_START_CAPACITY, allocator)
{
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray(const Array<T>& array, Allocator* allocator)
: DynamicArray(array.size < DYNAMIC_ARRAY_START_CAPACITY ? DYNAMIC_ARRAY_START_CAPACITY : array.size, allocator)
{
    FromArray(array);
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray(uint64 capacity, Allocator* allocator)
{
    size = 0;
    data = (T*)AllocateOrUseDefaultIfNull(allocator, capacity * sizeof(T));
    DEBUG_ASSERT(data != nullptr);
    this->capacity = capacity;
    this->allocator = allocator;
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::~DynamicArray()
{
    for (uint64 i = 0; i < size; i++) {
        data[i].~T();
    }
    FreeOrUseDefautIfNull(allocator, data);
}

template <typename T, typename Allocator>
Array<T>& DynamicArray<T, Allocator>::ToArray() const
{
    return *((Array<T>*)this);
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::FromArray(const Array<T>& array)
{
    if (capacity < array.size) {
        // TODO round to nearest power of 2?
        DEBUG_ASSERT(UpdateCapacity(array.size));
    }

    size = array.size;
    for (uint64 i = 0; i < size; i++) {
        data[i] = array.data[i];
    }
}

template <typename T, typename Allocator>
T* DynamicArray<T, Allocator>::Append()
{
    if (size >= capacity) {
        DEBUG_ASSERT(UpdateCapacity(capacity * 2));
    }

    new (&data[size]) T();
    return &data[size++];
}

template <typename T, typename Allocator>
T* DynamicArray<T, Allocator>::Append(const T& element)
{
    T* slot = Append();
    *slot = element;
    return slot;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Append(const Array<T>& array)
{
    Append((const Array<const T>)array);
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Append(const Array<const T>& array)
{
    uint64 newSize = size + array.size;
    if (capacity < newSize) {
        // TODO round to nearest power of 2?
        DEBUG_ASSERT(UpdateCapacity(newSize));
    }

    for (uint64 i = 0; i < array.size; i++) {
        data[size + i] = array.data[i];
    }
    size = newSize;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Clear()
{
    size = 0;
}

template <typename T, typename Allocator>
uint64 DynamicArray<T, Allocator>::IndexOf(const T& value)
{
    for (uint64 i = 0; i < size; i++) {
        if (data[i] == value) {
            return i;
        }
    }
    return size;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Free()
{
    FreeOrUseDefautIfNull(allocator, data);
}

template <typename T, typename Allocator>
inline T& DynamicArray<T, Allocator>::operator[](int index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, typename Allocator>
inline T& DynamicArray<T, Allocator>::operator[](uint64 index)
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, typename Allocator>
inline const T& DynamicArray<T, Allocator>::operator[](int index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, typename Allocator>
inline const T& DynamicArray<T, Allocator>::operator[](uint64 index) const
{
    ArrayBoundsCheck(index, size);
    return data[index];
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>& DynamicArray<T, Allocator>::operator=(const DynamicArray<T, Allocator>& other)
{
    FromArray(other.ToArray());
    return *this;
}

template <typename T, typename Allocator>
bool DynamicArray<T, Allocator>::UpdateCapacity(uint64 newCapacity)
{
    DEBUG_ASSERT(capacity != 0);
    DEBUG_ASSERT(newCapacity != 0);
    void* newMemory = ReAllocateOrUseDefaultIfNull(allocator, data, newCapacity * sizeof(T));
    if (newMemory == nullptr) {
        return false;
    }

    capacity = newCapacity;
    data = (T*)newMemory;
    return true;
}

HashKey::HashKey()
{
}

HashKey::HashKey(string str)
{
    WriteString(str);
}

HashKey::HashKey(const_string str)
{
    WriteString(str);
}

HashKey::HashKey(const char* str)
{
    WriteString(str);
}

bool HashKey::WriteString(const_string str)
{
    if (str.size > HASHKEY_MAX_LENGTH) {
        return false;
    }

    MemCopy(s.data, str.data, str.size * sizeof(char));
    s.size = str.size;
    return true;
}

bool HashKey::WriteString(const char* str)
{
    return WriteString(ToString(str));
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::HashTable()
: HashTable(nullptr)
{
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::HashTable(Allocator* allocator)
: HashTable(HASH_TABLE_START_CAPACITY, allocator)
{
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::HashTable(uint64 capacity, Allocator* allocator)
{
    size = 0;
    uint64 sizeBytes = sizeof(KeyValuePair<V>) * capacity;
    pairs = (KeyValuePair<V>*)allocator->Allocate(sizeBytes);
    if (pairs == nullptr) {
        DEBUG_PANIC("ERROR: not enough memory!\n");
    }

    for (uint64 i = 0; i < capacity; i++) {
        pairs[i].key.s.size = 0;
        new (&pairs[i]) KeyValuePair<V>();
    }

    this->capacity = capacity;
    this->allocator = allocator;
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::~HashTable()
{
    for (uint64 i = 0; i < capacity; i++) {
        pairs[i].~KeyValuePair<V>();
    }
    allocator->Free(pairs);
}

template <typename V, typename Allocator>
V* HashTable<V, Allocator>::Add(const HashKey& key)
{
    DEBUG_ASSERT(GetPair(key) == nullptr);

    if (size >= (uint64)((float32)capacity * HASH_TABLE_MAX_SIZE_TO_CAPACITY)) {
        uint64 newCapacity = NextPrime(capacity * 2);
        pairs = (KeyValuePair<V>*)allocator->ReAllocate(pairs, sizeof(KeyValuePair<V>) * newCapacity);
        if (pairs == nullptr) {
            DEBUG_PANIC("not enough memory for HashTable resize (pairs allocation)\n");
        }

        for (uint64 i = 0; i < capacity; i++) {
            // Don't placement new here, probably? Because it'll reset everything...
            // new (&pairs[i]) KeyValuePair<V>();
        }
        KeyValuePair<V>* oldPairs = (KeyValuePair<V>*)allocator->Allocate(sizeof(KeyValuePair<V>) * capacity);
        if (oldPairs == nullptr) {
            DEBUG_PANIC("not enough memory for HashTable resize (oldPairs allocation)\n");
        }
        defer(allocator->Free(oldPairs));
        MemCopy(oldPairs, pairs, sizeof(KeyValuePair<V>) * capacity);
        DEBUG_PANIC("TODO can't resize+rehash yet\n");
        // capacity = newCapacity;
    }

    KeyValuePair<V>* pair = GetFreeSlot(key);
    DEBUG_ASSERT(pair != nullptr);

    pair->key = key;
    size++;

    return &(pair->value);
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Add(const HashKey& key, const V& value)
{
    *(Add(key)) = value;
}

template <typename V, typename Allocator>
V* HashTable<V, Allocator>::GetValue(const HashKey& key)
{
    KeyValuePair<V>* pair = GetPair(key);
    if (pair == nullptr) {
        return nullptr;
    }

    return &pair->value;
}

template <typename V, typename Allocator>
const V* HashTable<V, Allocator>::GetValue(const HashKey& key) const
{
    const KeyValuePair<V>* pair = GetPair(key);
    if (pair == nullptr) {
        return nullptr;
    }

    return &pair->value;
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Clear()
{
    for (uint64 i = 0; i < capacity; i++) {
        pairs[i].key.s.size = 0;
    }
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Free()
{
    allocator->Free(pairs);

    capacity = 0;
    size = 0;
}

template <typename V, typename Allocator>
HashTable<V, Allocator>& HashTable<V, Allocator>::operator=(const HashTable<V, Allocator>& other)
{
    DEBUG_ASSERT(capacity == other.capacity); // TODO no rehashing, so we do same-capacity only
    size = other.size;
    for (uint64 i = 0; i < capacity; i++) {
        pairs[i] = other.pairs[i];
    }
    return *this;
}

template <typename V, typename Allocator>
KeyValuePair<V>* HashTable<V, Allocator>::GetPair(const HashKey& key) const
{
    uint64 hashInd = KeyHash(key) % capacity;
    for (uint64 i = 0; i < capacity; i++) {
        KeyValuePair<V>* pair = pairs + hashInd + i;
        if (KeyCompare(pair->key, key)) {
            return pair;
        }
        if (pair->key.s.size == 0) {
            return nullptr;
        }
    }

    return nullptr;
}

template <typename V, typename Allocator>
KeyValuePair<V>* HashTable<V, Allocator>::GetFreeSlot(const HashKey& key)
{
    uint64 hashInd = KeyHash(key) % capacity;
    for (uint64 i = 0; i < capacity; i++) {
        KeyValuePair<V>* pair = pairs + hashInd + i;
        if (pair->key.s.size == 0) {
            return pair;
        }
    }

    return nullptr;
}
