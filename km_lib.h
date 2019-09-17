#pragma once

#include "km_defines.h"

#define C_ARRAY_LENGTH(cArray) (sizeof(cArray) / sizeof(cArray[0]))

#define STRING_KEY_MAX_LENGTH 64

int ToIntOrTruncate(uint64 n);

void MemCopy(void* dst, const void* src, uint64 numBytes);
void MemMove(void* dst, const void* src, uint64 numBytes);
void MemSet(void* dst, char value, uint64 numBytes);

// defer C++11 implementation, source: https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct _DeferFunctionObject
{
    F function;
    _DeferFunctionObject(F function) : function(function) {}
    ~_DeferFunctionObject() { function(); }
};

template <typename F>
_DeferFunctionObject<F> _DeferFunction(F function)
{
    return _DeferFunctionObject<F>(function);
}

#define STRING_JOIN2(x, y) (x##y)
#define defer(code) auto STRING_JOIN2(_defer_, __COUNTER__) = _DeferFunction([&]() { code; })

template <typename T>
struct Array
{
	uint64 size;
	T* data;

	void Append(const T& element);
	void RemoveLast();

	// Slow, linear time
	void Remove(uint64 index);
	void AppendAfter(const T& element, uint64 index);
	
	inline T operator[](int index) const;
	inline T operator[](uint64 index) const;
	inline T& operator[](int index);
	inline T& operator[](uint64 index);
};

template <typename T, uint64 S>
struct FixedArray
{
	T fixedArray[S];
	Array<T> array;

	void Init(); // TODO ew, shouldn't need this

	void Append(const T& element);
	void RemoveLast();

	// Slow, linear time
	void Remove(uint64 index);
	void AppendAfter(const T& element, uint64 index);
	
	inline T operator[](int index) const;
	inline T operator[](uint64 index) const;
	inline T& operator[](int index);
	inline T& operator[](uint64 index);

	inline void operator=(const FixedArray<T, S>& other);
};

template <typename T>
struct DynamicArray
{
	uint64 capacity;
	Array<T> array;

	void Allocate();
	void Allocate(uint64 capacity);
	void Free();

	void Append(const T& element);
	void RemoveLast();

	// Slow, linear time
	void Remove(uint64 index);
	void AppendAfter(const T& element, uint64 index);
	
	inline T operator[](int index) const;
	inline T operator[](uint64 index) const;
	inline T& operator[](int index);
	inline T& operator[](uint64 index);
};

struct HashKey
{
	FixedArray<char, STRING_KEY_MAX_LENGTH> string;

	HashKey();
	HashKey(const Array<char>& str);
	HashKey(const char* str);

	void WriteString(const Array<char>& str);
	void WriteString(const char* str);
};

template <typename V>
struct KeyValuePair
{
	HashKey key;
	V value;
};

template <typename V>
struct HashTable
{
	uint64 size;
	uint64 capacity;

	KeyValuePair<V>* pairs;

	void Init();
	void Init(uint64 capacity);

	void Add(const HashKey& key, V value);
	V* GetValue(const HashKey& key) const; // const... sure, if it helps you sleep at night
	bool32 Remove(const HashKey& key);

	void Clear();
	void Free();

private:
	KeyValuePair<V>* GetPair(const HashKey& key) const;
	KeyValuePair<V>* GetFreeSlot(const HashKey& key);
};

bool32 KeyCompare(const HashKey& key1, const HashKey& key2);