#pragma once

#include "km_lib.h"

// TODO pretty random, but ok
#define PATH_MAX_LENGTH 256

uint64 StringLength(const char* str);

Array<char> ToString(const char* cString);
template <uint64 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString);
template <typename Allocator>
char* ToCString(const Array<char>& string, Allocator* allocator);

bool StringCompare(const Array<char>& str1, const Array<char>& str2);
bool StringCompare(const Array<char>& str1, const char* str2);
bool StringCompare(const char* str1, const char* str2);
bool StringContains(const Array<char>& str, const char* substring);
void CatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest);
void StringCat(const char* str1, const char* str2, char* dest, uint64 destMaxLength);

inline bool IsWhitespace(char c);
void TrimWhitespace(const Array<char>& string, Array<char>* trimmed);
bool StringToIntBase10(const Array<char>& string, int* intBase10);
bool StringToUInt64Base10(const Array<char>& string, uint64* intBase10);
bool StringToFloat32(const Array<char>& string, float32* f);
uint64 GetLastOccurrence(const Array<char>& string, char c);
void ReadElementInSplitString(Array<char>* element, Array<char>* next, char separator);

template <typename Allocator>
bool Utf8ToUppercase(const Array<char>& utf8String, DynamicArray<char, Allocator>* outString);

template <typename Allocator>
Array<char> AllocPrintf(Allocator* allocator, const char* format, ...);

template <typename T>
bool StringToElementArray(const Array<char>& string, char sep, bool trimElements,
	bool (*conversionFunction)(const Array<char>&, T*),
	int maxElements, T* array, int* numElements);

// TODO this fits more into a km-file-format module. not really a general string lib function
template <uint64 KEYWORD_SIZE, uint64 VALUE_SIZE>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, FixedArray<char, VALUE_SIZE>* outValue);
