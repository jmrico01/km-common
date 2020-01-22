#pragma once

#include "km_defines.h"
#include "km_memory.h"

#define C_ARRAY_LENGTH(cArray) (sizeof(cArray) / sizeof(cArray[0]))

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
	uint64 IndexOf(const T& value);

	Array<T> Slice(uint64 start, uint64 end) const;
	Array<T> SliceTo(uint64 end) const;
	Array<T> SliceFrom(uint64 start) const;

	inline T& operator[](int index);
	inline T& operator[](uint64 index);
	inline const T& operator[](int index) const;
	inline const T& operator[](uint64 index) const;
};

template <typename T, uint64 S>
struct FixedArray
{
	uint64 size;
	T data[S];

	Array<T> ToArray() const; // NOTE modifying this array's size won't affect the FixedArray size

	T* Append();
	void Append(const T& element);
	void Append(const Array<T>& array);
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

// TODO Assignment (=) of DynamicArray shallow-copies their members. Override the operator, probably
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

	Array<T>& ToArray();
	const Array<T>& ToArray() const;
	void FromArray(const Array<T>& array);

	T* Append();
	void Append(const T& element);
	void Append(const Array<T>& array);
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
	FixedArray<char, HASHKEY_MAX_LENGTH> string;

	HashKey();
	HashKey(const Array<char>& str);
	HashKey(const char* str);

	bool WriteString(const Array<char>& str);
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
	bool32 Remove(const HashKey& key);

	void Clear();
	void Free();

	HashTable<V, Allocator>& operator=(const HashTable<V, Allocator>& other);

private:
	KeyValuePair<V>* GetPair(const HashKey& key) const;
	KeyValuePair<V>* GetFreeSlot(const HashKey& key);
};

bool32 KeyCompare(const HashKey& key1, const HashKey& key2);
