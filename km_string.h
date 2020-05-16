#pragma once

#include "km_lib.h"

using string = Array<char>;
using const_string = const Array<const char>;

// TODO pretty random, but ok
#define PATH_MAX_LENGTH 256

uint64 StringLength(const char* str);

const_string ToString(const char* cString);
string ToNonConstString(const_string constString); // TODO(patio): wish I didn't have to do this, maybe?

#ifdef KM_CPP_STD
#include <string>
Array<char> ToString(const std::string& string);
#endif
template <uint64 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString);
template <typename Allocator>
char* ToCString(const_string string, Allocator* allocator);

int StringCompare(const_string str1, const_string str2);
bool StringEquals(const_string str1, const_string str2);

void CatStrings(size_t sourceACount, const char* sourceA,
                size_t sourceBCount, const char* sourceB,
                size_t destCount, char* dest);
void StringCat(const char* str1, const char* str2, char* dest, uint64 destMaxLength);

uint64 SubstringSearch(const_string string, const_string substring);
bool StringContains(const_string string, const_string substring);

bool IsNewline(char c);
bool IsWhitespace(char c);
bool IsAlphanumeric(char c);
Array<char> TrimWhitespace(const Array<char>& string);
void TrimWhitespace(const Array<char>& string, Array<char>* trimmed);
bool StringToIntBase10(const Array<char>& string, int* intBase10);
bool StringToUInt64Base10(const Array<char>& string, uint64* intBase10);
bool StringToFloat32(const Array<char>& string, float32* f);

template <typename Allocator>
void StringSplit(const Array<char>& string, char c, DynamicArray<Array<char>, Allocator>* outSplit);
Array<char> NextSplitElement(Array<char>* string, char separator);
void ReadElementInSplitString(Array<char>* element, Array<char>* next, char separator);

template <typename Allocator>
string AllocPrintf(Allocator* allocator, const char* format, ...);
template <typename Allocator>
DynamicArray<char, Allocator> AllocPrintf(const char* format, ...);

template <typename T>
bool StringToElementArray(const Array<char>& string, char sep, bool trimElements,
                          bool (*conversionFunction)(const Array<char>&, T*),
                          int maxElements, T* array, int* numElements);

#ifdef KM_UTF8
template <typename Allocator>
bool Utf8ToUppercase(const Array<char>& utf8String, DynamicArray<char, Allocator>* outString);
#endif
