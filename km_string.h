#pragma once

#include "km_lib.h"

// TODO plz standardize file paths
#define PATH_MAX_LENGTH 128

uint64 StringLength(const char* str);

Array<char> ToString(const char* cString);
void InitFromCString(Array<char>* string, const char* cString);
template <uint64 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString);

bool StringCompare(const Array<char>& str1, const Array<char>& str2);
bool StringCompare(const Array<char>& str1, const char* str2);
bool StringCompare(const char* str1, const char* str2);
bool StringContains(const Array<char>& str, const char* substring);
void CatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest);
void StringCat(const char* str1, const char* str2, char* dest, uint64 destMaxLength);
template <uint64 S>
bool StringAppend(FixedArray<char, S>* string, const char* toAppend);
template <uint64 S>
bool StringAppend(FixedArray<char, S>* string, const Array<char>& toAppend);

inline bool32 IsWhitespace(char c);
void TrimWhitespace(const Array<char>& string, Array<char>* trimmed);
bool32 StringToIntBase10(const Array<char>& string, int* intBase10);
bool32 StringToUInt64Base10(const Array<char>& string, uint64* intBase10);
bool32 StringToFloat32(const Array<char>& string, float32* f);
uint64 GetLastOccurrence(const Array<char>& string, char c);
void ReadElementInSplitString(Array<char>* element, Array<char>* next, char separator);

template <typename Allocator>
Array<char> AllocPrintf(Allocator* allocator, const char* format, ...);

template <typename T>
bool32 StringToElementArray(const Array<char>& string, char sep, bool trimElements,
	bool32 (*conversionFunction)(const Array<char>&, T*),
	int maxElements, T* array, int* numElements);

// TODO this fits more into a km-file-format module. not really a general string lib function
template <uint64 KEYWORD_SIZE, uint64 VALUE_SIZE>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, FixedArray<char, VALUE_SIZE>* outValue);
