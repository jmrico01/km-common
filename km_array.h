#pragma once

#include "km_defines.h"

template <typename T>
struct Array
{
    const static Array<T> empty;

    uint32 size;
    T* data;

    void RemoveLast();
    void Clear();
    uint32 FindFirst(const T& value, uint32 start = 0) const;
    uint32 FindLast(const T& value) const;

    Array<T> Slice(uint32 start, uint32 end) const;
    Array<T> SliceTo(uint32 end) const;
    Array<T> SliceFrom(uint32 start) const;

    void CopyFrom(const Array<T>& other);
    void Shuffle();

    inline T& operator[](uint32 index);
    inline const T& operator[](uint32 index) const;

    operator const Array<const T>() const;
};

template <typename T, uint32 S>
struct StaticArray
{
    static const uint32 SIZE = S;

    T data[S];

    Array<T> ToArray() const;

    inline T& operator[](uint32 index);
    inline const T& operator[](uint32 index) const;

    StaticArray<T, S>& operator=(const StaticArray<T, S>& other);
};

template <typename T, uint32 S>
struct FixedArray
{
    static const uint32 MAX_SIZE = S;

    uint32 size;
    T data[S];

    Array<T> ToArray() const; // NOTE modifying the returned array's size won't affect the FixedArray size
    const Array<const T> ToConstArray() const;
    void FromArray(const Array<T>& array);
    void FromArray(const Array<const T>& array);

    T* Append();
    T* Append(const T& element);
    void Append(const Array<T>& array);
    void Append(const Array<const T>& array);
    void RemoveLast();
    void Clear();

    // slow, linear time
    void AppendAfter(const T& element, uint32 index);
    void Remove(uint32 index);

    inline T& operator[](uint32 index);
    inline const T& operator[](uint32 index) const;

    FixedArray<T, S>& operator=(const FixedArray<T, S>& other);
};

template <typename T>
struct LargeArray
{
    const static LargeArray<T> empty;

    uint64 size;
    T* data;

    void RemoveLast();
    void Clear();

    LargeArray<T> Slice(uint64 start, uint64 end) const;
    LargeArray<T> SliceTo(uint64 end) const;
    LargeArray<T> SliceFrom(uint64 start) const;

    inline T& operator[](uint64 index);
    inline const T& operator[](uint64 index) const;

    operator const LargeArray<const T>() const;
};
