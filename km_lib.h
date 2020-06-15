#pragma once

#include "km_defines.h"

// https://stackoverflow.com/questions/4415524/common-array-length-macro-for-c
#define C_ARRAY_LENGTH(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

const uint64 HASHKEY_MAX_LENGTH = 64;

const uint8  UINT8_MAX_VALUE  = 0xff;
const uint16 UINT16_MAX_VALUE = 0xffff;
const uint32 UINT32_MAX_VALUE = 0xffffffff;
const uint64 UINT64_MAX_VALUE = 0xffffffffffffffff;

int ToIntOrTruncate(uint64 n);
uint32 SafeTruncateUInt64(uint64 value);

void MemCopy(void* dst, const void* src, uint64 numBytes);
void MemMove(void* dst, const void* src, uint64 numBytes);
void MemSet(void* dst, char value, uint64 numBytes);
int  MemComp(const void* mem1, const void* mem2, uint64 numBytes);

// defer C++11 implementation, source: https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct _DeferFunctionObject {
    F function;
    _DeferFunctionObject(F function) : function(function) {}
    ~_DeferFunctionObject() { function(); }
};

template <typename F>
_DeferFunctionObject<F> _DeferFunction(F function) {
    return _DeferFunctionObject<F>(function);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = _DeferFunction([&](){code;})

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
