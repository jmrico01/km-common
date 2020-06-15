#include "km_lib.h"

#include <cstring> // memcpy is here... no idea why
#include <limits.h>

#include "km_math.h"
#include "km_string.h"

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
