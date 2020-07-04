#pragma once

#include "km_array.h"
#include "km_container.h"

using string = Array<char>;
using const_string = const Array<const char>;

// TODO pretty random, but ok
#define PATH_MAX_LENGTH 256

uint32 StringLength(const char* str);

const_string ToString(const char* cString);
string ToNonConstString(const_string constString); // TODO(patio): wish I didn't have to do this, maybe?
#ifdef KM_CPP_STD
#include <string>
string ToString(const std::string& str);
#endif

template <uint32 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString);
template <typename Allocator>
char* ToCString(const_string str, Allocator* allocator);

int StringCompare(const_string str1, const_string str2);
bool StringEquals(const_string str1, const_string str2);

uint32 SubstringSearch(const_string str, const_string substr);
bool StringContains(const_string str, const_string substr);

bool IsNewline(char c);
bool IsWhitespace(char c);
bool IsAlphanumeric(char c);

string TrimWhitespace(const_string str);

bool StringToIntBase10(const_string str, int* intBase10);
bool StringToUInt32Base10(const_string str, uint32* intBase10);
bool StringToFloat32(const_string str, float32* f);

string NextSplitElement(string* str, char separator);
template <typename Allocator>
void StringSplit(const_string str, char c, DynamicArray<string, Allocator>* outSplit);

template <typename T>
bool StringToElementArray(const_string str, char sep, bool trimElements,
                          bool (*conversionFunction)(const_string, T*),
                          int maxElements, T* array, int* numElements);

template <typename Allocator>
string AllocPrintf(Allocator* allocator, const char* format, ...);
template <typename Allocator>
DynamicArray<char, Allocator> AllocPrintf(const char* format, ...);

#ifdef KM_UTF8
template <typename Allocator>
bool Utf8ToUppercase(const_string utf8String, DynamicArray<char, Allocator>* outString);
#endif
