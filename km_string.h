#pragma once

#include "km_lib.h"

// TODO I could be even more strict/precise about const in these functions.
// Typically, when I say "const Array<char>", I really mean "Array<const char>".
// Not sure how pedantic that will get, and how much it will affect readability of usage code.

// TODO pretty random, but ok
#define PATH_MAX_LENGTH 256

uint64 StringLength(const char* str);

Array<char> ToString(const char* cString);
#ifdef KM_CPP_STD
#include <string>
Array<char> ToString(const std::string& string);
#endif
template <uint64 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString);
template <typename Allocator>
char* ToCString(const Array<char>& string, Allocator* allocator);

int StringCompare(const Array<char>& str1, const Array<char>& str2);
bool StringEquals(const Array<char>& str1, const Array<char>& str2);

void CatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest);
void StringCat(const char* str1, const char* str2, char* dest, uint64 destMaxLength);

uint64 SubstringSearch(const Array<char>& string, const Array<char>& substring);
bool StringContains(const Array<char>& string, const Array<char>& substring);

bool IsNewline(char c);
bool IsWhitespace(char c);
bool IsAlphanumeric(char c);
void TrimWhitespace(const Array<char>& string, Array<char>* trimmed);
bool StringToIntBase10(const Array<char>& string, int* intBase10);
bool StringToUInt64Base10(const Array<char>& string, uint64* intBase10);
bool StringToFloat32(const Array<char>& string, float32* f);

template <typename Allocator>
void StringSplit(const Array<char>& string, char c, DynamicArray<Array<char>, Allocator>* outSplit);
void ReadElementInSplitString(Array<char>* element, Array<char>* next, char separator);

template <typename Allocator>
Array<char> AllocPrintf(Allocator* allocator, const char* format, ...);

template <typename T>
bool StringToElementArray(const Array<char>& string, char sep, bool trimElements,
	bool (*conversionFunction)(const Array<char>&, T*),
	int maxElements, T* array, int* numElements);

#ifdef KM_UTF8
template <typename Allocator>
bool Utf8ToUppercase(const Array<char>& utf8String, DynamicArray<char, Allocator>* outString);
#endif
