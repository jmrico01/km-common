#include "km_array.h"

#define ARRAY_BOUNDS_CHECK(index, size) DEBUG_ASSERTF(0 <= index && (uint64)index < size, \
"Array bounds check failed: index %" PRIu64 ", size %" PRIu64 "\n", \
(uint64)index, size)

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
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
inline T& Array<T>::operator[](uint64 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](int index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint64 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint64 S>
Array<T> StaticArray<T, S>::ToArray() const
{
    return {
        .size = S,
        .data = (T*)data
    };
}

template <typename T, uint64 S>
inline T& StaticArray<T, S>::operator[](int index)
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint64 S>
inline T& StaticArray<T, S>::operator[](uint64 index)
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint64 S>
inline const T& StaticArray<T, S>::operator[](int index) const
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint64 S>
inline const T& StaticArray<T, S>::operator[](uint64 index) const
{
    ARRAY_BOUNDS_CHECK(index, S);
    return data[index];
}

template <typename T, uint64 S>
StaticArray<T, S>& StaticArray<T, S>::operator=(const StaticArray<T, S>& other)
{
    MemCopy(data, other.data, S * sizeof(T));
    return *this;
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
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline T& FixedArray<T, S>::operator[](uint64 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline const T& FixedArray<T, S>::operator[](int index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, uint64 S>
inline const T& FixedArray<T, S>::operator[](uint64 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
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
