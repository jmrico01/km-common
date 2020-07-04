#include "km_array.h"

#define ARRAY_BOUNDS_CHECK(index, size) DEBUG_ASSERTF(0 <= index && (uint32)index < size, \
"Array bounds check failed: index %" PRIu32 ", size %" PRIu32 "\n", \
(uint32)index, size)

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
uint32 Array<T>::FindFirst(const T& value, uint32 start) const
{
    for (uint32 i = start; i < size; i++) {
        if (data[i] == value) {
            return i;
        }
    }
    return size;
}

template <typename T>
uint32 Array<T>::FindLast(const T& value) const
{
    for (uint32 i = size; i != 0; i--) {
        if (data[i - 1] == value) {
            return i - 1;
        }
    }
    return size;
}

template <typename T>
Array<T> Array<T>::Slice(uint32 start, uint32 end) const
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
Array<T> Array<T>::SliceTo(uint32 end) const
{
    return Slice(0, end);
}

template <typename T>
Array<T> Array<T>::SliceFrom(uint32 start) const
{
    return Slice(start, size);
}

template <typename T>
void Array<T>::Shuffle()
{
    for (uint32 i = size; i != 0; i--) {
        uint32 j = RandUInt32(i);
        T temp = data[i - 1];
        data[i - 1] = data[j];
        data[j] = temp;
    }
}

template <typename T>
inline T& Array<T>::operator[](uint32 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint32 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
Array<T>::operator const Array<const T>() const
{
    return *((const Array<const T>*)this);
}

template <typename T, uint32 S>
Array<T> StaticArray<T, S>::ToArray() const
{
    return {
        .size = S,
        .data = (T*)data
    };
}

template <typename T, uint32 S>
inline T& StaticArray<T, S>::operator[](uint32 index)
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint32 S>
inline const T& StaticArray<T, S>::operator[](uint32 index) const
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint32 S>
StaticArray<T, S>& StaticArray<T, S>::operator=(const StaticArray<T, S>& other)
{
    MemCopy(data, other.data, S * sizeof(T));
    return *this;
}

template <typename T, uint32 S>
Array<T> FixedArray<T, S>::ToArray() const
{
    return {
        .size = size,
        .data = (T*)data
    };
}

template <typename T, uint32 S>
const Array<const T> FixedArray<T, S>::ToConstArray() const
{
    return {
        .size = size,
        .data = (T*)data
    };
}

template <typename T, uint32 S>
void FixedArray<T, S>::FromArray(const Array<T>& array)
{
    DEBUG_ASSERT(array.size <= S);
    MemCopy(data, array.data, array.size * sizeof(T));
    size = array.size;
}

template <typename T, uint32 S>
T* FixedArray<T, S>::Append()
{
    DEBUG_ASSERTF(size < S, "size %" PRIu32 ", S %" PRIu32 "\n", size, S);
    return &data[size++];
}

template <typename T, uint32 S>
T* FixedArray<T, S>::Append(const T& element)
{
    T* slot = Append();
    *slot = element;
    return slot;
}

template <typename T, uint32 S>
void FixedArray<T, S>::Append(const Array<T>& array)
{
    Append((const Array<const T>)array);
}

template <typename T, uint32 S>
void FixedArray<T, S>::Append(const Array<const T>& array)
{
    uint32 newSize = size + array.size;
    DEBUG_ASSERTF(newSize <= S, "size %" PRIu32 ", S %" PRIu32 ", array.size %" PRIu32 "\n",
                  size, S, array.size);

    for (uint32 i = 0; i < array.size; i++) {
        data[size + i] = array.data[i];
    }
    size = newSize;
}

template <typename T, uint32 S>
void FixedArray<T, S>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T, uint32 S>
void FixedArray<T, S>::Clear()
{
    size = 0;
}

template <typename T, uint32 S>
void FixedArray<T, S>::AppendAfter(const T& element, uint32 index)
{
    DEBUG_ASSERT(index < size);
    DEBUG_ASSERTF(size < S, "size %" PRIu64 ", S %" PRIu64 "\n", size, S);

    uint32 targetIndex = index + 1;
    for (uint32 i = size; i > targetIndex; i--) {
        data[i] = data[i - 1];
    }
    data[targetIndex] = element;
    size++;
}

template <typename T, uint32 S>
void FixedArray<T, S>::Remove(uint32 index)
{
    DEBUG_ASSERT(size > 0);
    DEBUG_ASSERT(index < size);
    for (uint32 i = index + 1; i < size; i++) {
        data[i - 1] = data[i];
    }
    size--;
}

template <typename T, uint32 S>
inline T& FixedArray<T, S>::operator[](uint32 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint32 S>
inline const T& FixedArray<T, S>::operator[](uint32 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint32 S>
FixedArray<T, S>& FixedArray<T, S>::operator=(const FixedArray<T, S>& other)
{
    DEBUG_ASSERT(other.size <= S);
    size = other.size;
    MemCopy(data, other.data, size * sizeof(T));
    return *this;
}

// LargeArray

template <typename T> const LargeArray<T> LargeArray<T>::empty = { .size = 0, .data = nullptr };

template <typename T>
void LargeArray<T>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T>
void LargeArray<T>::Clear()
{
    size = 0;
}

template <typename T>
LargeArray<T> LargeArray<T>::Slice(uint64 start, uint64 end) const
{
    DEBUG_ASSERT(start < size);
    DEBUG_ASSERT(end <= size);
    DEBUG_ASSERT(start <= end);
    LargeArray<T> slice;
    slice.data = &data[start];
    slice.size = end - start;
    return slice;
}

template <typename T>
LargeArray<T> LargeArray<T>::SliceTo(uint64 end) const
{
    return Slice(0, end);
}

template <typename T>
LargeArray<T> LargeArray<T>::SliceFrom(uint64 start) const
{
    return Slice(start, size);
}

template <typename T>
inline T& LargeArray<T>::operator[](uint64 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
inline const T& LargeArray<T>::operator[](uint64 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
LargeArray<T>::operator const LargeArray<const T>() const
{
    return *((const LargeArray<const T>*)this);
}