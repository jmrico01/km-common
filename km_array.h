#pragma once

#include "km_defines.h"

template <typename T>
struct Array
{
    const static Array<T> empty;

    uint64 size;
    T* data;

    void RemoveLast();
    void Clear();
    uint64 FindFirst(const T& value, uint64 start = 0) const;
    uint64 FindLast(const T& value) const;

    Array<T> Slice(uint64 start, uint64 end) const;
    Array<T> SliceTo(uint64 end) const;
    Array<T> SliceFrom(uint64 start) const;

    inline T& operator[](int index);
    inline T& operator[](uint64 index);
    inline const T& operator[](int index) const;
    inline const T& operator[](uint64 index) const;

    operator const Array<const T>() const { return *((const Array<const T>*)this); }
};

template <typename T, uint64 S>
struct FixedArray
{
    uint64 size;
    T data[S];

    Array<T> ToArray() const; // NOTE modifying the returned array's size won't affect the FixedArray size
    const Array<const T> ToConstArray() const;
    void FromArray(const Array<T>& array);

    T* Append();
    T* Append(const T& element);
    void Append(const Array<T>& array);
    void Append(const Array<const T>& array);
    void RemoveLast();
    void Clear();
    uint64 IndexOf(const T& value);

    // slow, linear time
    void AppendAfter(const T& element, uint64 index);
    void Remove(uint64 index);

    inline T& operator[](int index);
    inline T& operator[](uint64 index);
    inline const T& operator[](int index) const;
    inline const T& operator[](uint64 index) const;

    FixedArray<T, S>& operator=(const FixedArray<T, S>& other);
};
