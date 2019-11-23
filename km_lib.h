#pragma once

#include "km_defines.h"
#include "km_memory.h"

#define C_ARRAY_LENGTH(cArray) (sizeof(cArray) / sizeof(cArray[0]))

const uint64 STRING_KEY_MAX_LENGTH = 64;

const uint8  UINT8_MAX_VALUE  = 0xff;
const uint16 UINT16_MAX_VALUE = 0xffff;
const uint32 UINT32_MAX_VALUE = 0xffffffff;
const uint64 UINT64_MAX_VALUE = 0xffffffffffffffff;

int ToIntOrTruncate(uint64 n);

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
	uint64 size;
	T* data;

	T* Append();
	void Append(const T& element);
	void RemoveLast();

	Array<T> Slice(uint64 start, uint64 end) const;
	Array<T> SliceTo(uint64 end) const;
	Array<T> SliceFrom(uint64 start) const;

	// slow, linear time
	void AppendAfter(const T& element, uint64 index);
	void Remove(uint64 index);

	inline T& operator[](int index);
	inline T& operator[](uint64 index);
	inline const T& operator[](int index) const;
	inline const T& operator[](uint64 index) const;
};

template <typename T, uint64 S>
struct FixedArray
{
	T fixedArray[S];
	Array<T> array;

	void Init(); // TODO ew, shouldn't need this

	T* Append();
	void Append(const T& element);
	void RemoveLast();

	// slow, linear time
	void AppendAfter(const T& element, uint64 index);
	void Remove(uint64 index);

	inline T& operator[](int index);
	inline T& operator[](uint64 index);
	inline const T& operator[](int index) const;
	inline const T& operator[](uint64 index) const;

	FixedArray<T, S>& operator=(const FixedArray<T, S>& other);
};

template <typename T, typename Allocator = StandardAllocator>
struct DynamicArray // TODO figure out where allocator will go
{
	Array<T> array;
	uint64 capacity;
	Allocator* allocator;

	DynamicArray(uint64 capacity, Allocator* allocator = nullptr);
	DynamicArray(Allocator* allocator = nullptr);

	T* Append();
	void Append(const T& element);
	void RemoveLast();
	void Clear();
	void Free();

	inline T& operator[](int index);
	inline T& operator[](uint64 index);
	inline const T& operator[](int index) const;
	inline const T& operator[](uint64 index) const;

	void UpdateCapacity(uint64 newCapacity);
};

template <typename T, typename Allocator = StandardAllocator>
struct DynamicQueue
{
	uint64 start, end;
	uint64 capacity;
	Allocator* allocator;
	T* data;

	DynamicQueue(uint64 capacity, Allocator* allocator = nullptr);
	DynamicQueue(Allocator* allocator = nullptr);

	void Append(const T& element);
	const T& GetFirst();
	void RemoveFirst();
	bool IsEmpty();
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
	V* GetValue(const HashKey& key);
	const V* GetValue(const HashKey& key) const;
	bool32 Remove(const HashKey& key);

	void Clear();
	void Free();

private:
	KeyValuePair<V>* GetPair(const HashKey& key) const;
	KeyValuePair<V>* GetFreeSlot(const HashKey& key);
};

bool32 KeyCompare(const HashKey& key1, const HashKey& key2);
