#include "km_string.h"

#include "km_debug.h"
#include "km_math.h"

uint64 StringLength(const char* string)
{
	int length = 0;
	while (*string++) {
		length++;
	}

	return length;
}

Array<char> ToString(const char* cString)
{
	return {
		.size = StringLength(cString),
		.data = (char*)cString
	};
}

void InitFromCString(Array<char>* string, const char* cString)
{
	string->data = (char*)cString; // NOTE oof
	string->size = StringLength(cString);
}

template <uint64 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString)
{
	uint64 stringLength = StringLength(cString);
	if (stringLength > S) {
		stringLength = S;
	}
	string->array.size = stringLength;
	MemCopy(string->fixedArray, cString, stringLength * sizeof(char));
}

bool StringCompare(const char* str1, const char* str2, uint64 n)
{
	for (uint64 i = 0; i < n; i++) {
		if (str1[i] != str2[i]) {
			return false;
		}
	}

	return true;
}

bool StringCompare(const Array<char>& str1, const Array<char>& str2)
{
    return StringCompare(str1.data, str2.data, MaxUInt64(str1.size, str2.size));
}

bool StringCompare(const Array<char>& str1, const char* str2)
{
    return StringCompare(str1.data, str2, MaxUInt64(str1.size, StringLength(str2)));
}

bool StringCompare(const char* str1, const char* str2)
{
    return StringCompare(str1, str2, MaxUInt64(StringLength(str1), StringLength(str2)));
}

bool StringContains(const Array<char>& str, const char* substring)
{
	for (uint64 i = 0; i < str.size; i++) {
		char c = *substring++;
		if (c == '\0') {
			return true;
		}
		if (c != str[i]) {
			return false;
		}
	}

	return false;
}

void CatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest)
{
	for (size_t i = 0; i < sourceACount; i++) {
		*dest++ = *sourceA++;
	}

	for (size_t i = 0; i < sourceBCount; i++) {
		*dest++ = *sourceB++;
	}

	*dest++ = '\0';
}

void StringCat(const char* str1, const char* str2, char* dest, uint64 destMaxLength)
{
	CatStrings(StringLength(str1), str1, StringLength(str2), str2, destMaxLength, dest);
}

template <uint64 S>
bool StringAppend(FixedArray<char, S>* string, const char* toAppend)
{
	Array<char> toAppendString;
	InitFromCString(&toAppendString, toAppend);
	return StringAppend(string, toAppendString);
}

template <uint64 S>
bool StringAppend(FixedArray<char, S>* string, const Array<char>& toAppend)
{
	uint64 newSize = string->array.size + toAppend.size;
	if (newSize > S) {
		return false;
	}
	MemCopy(string->array.data + string->array.size, toAppend.data, toAppend.size * sizeof(char));
	string->array.size = newSize;
	return true;
}

inline bool32 IsWhitespace(char c)
{
	return c == ' ' || c == '\t'
		|| c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

void TrimWhitespace(const Array<char>& string, Array<char>* trimmed)
{
	uint64 start = 0;
	while (start < string.size && IsWhitespace(string[start])) {
		start++;
	}
	uint64 end = string.size;
	while (end > 0 && IsWhitespace(string[end - 1])) {
		end--;
	}

	trimmed->data = string.data + start;
	trimmed->size = end - start;
}

bool32 StringToIntBase10(const Array<char>& string, int* intBase10)
{
	if (string.size == 0) {
		return false;
	}

	bool32 negative = false;
	*intBase10 = 0;
	for (uint64 i = 0; i < string.size; i++) {
		char c = string[i];
		if (i == 0 && c == '-') {
			negative = true;
			continue;
		}
		if (c < '0' || c > '9') {
			return false;
		}
		*intBase10 = (*intBase10) * 10 + (int)(c - '0');
	}

	if (negative) {
		*intBase10 = -(*intBase10);
	}
	return true;
}

bool32 StringToUInt64Base10(const Array<char>& string, uint64* intBase10)
{
    if (string.size == 0) {
        return false;
    }

    *intBase10 = 0;
    for (uint64 i = 0; i < string.size; i++) {
        char c = string[i];
        *intBase10 = (*intBase10) * 10 + (uint64)(c - '0');
    }

    return true;
}

bool32 StringToFloat32(const Array<char>& string, float32* f)
{
	uint64 dotIndex = 0;
	while (dotIndex < string.size && string[dotIndex] != '.') {
		dotIndex++;
	}

	int whole = 0;
	float32 wholeNegative = false;
	if (dotIndex > 0) {
		Array<char> stringWhole;
		stringWhole.size = dotIndex;
		stringWhole.data = string.data;
		if (!StringToIntBase10(stringWhole, &whole)) {
			return false;
		}
		wholeNegative = string[0] == '-';
	}
	int frac = 0;
	Array<char> fracString;
	fracString.size = string.size - dotIndex - 1;
	if (fracString.size > 0) {
		fracString.data = string.data + dotIndex + 1;
		if (!StringToIntBase10(fracString, &frac)) {
			return false;
		}
	}

	*f = (float32)whole;
	if (fracString.size > 0) {
		frac = wholeNegative ? -frac : frac;
		float32 fractional = (float32)frac;
		for (uint64 i = 0; i < fracString.size; i++) {
			fractional /= 10.0f;
		}
		*f += fractional;
	}
	return true;
}

uint64 GetLastOccurrence(const Array<char>& string, char c)
{
	for (uint64 i = string.size; i != 0; i--) {
		if (string[i - 1] == c) {
			return i;
		}
	}

	return string.size;
}

void ReadElementInSplitString(Array<char>* element, Array<char>* next, char separator)
{
	for (uint64 i = 0; i < element->size; i++) {
		if ((*element)[i] == separator) {
			next->data = element->data + i + 1;
			next->size = element->size - i - 1;
			element->size = i;
			return;
		}
	}

	next->size = 0;
}

template <typename Allocator>
Array<char> AllocPrintf(Allocator* allocator, const char* format, ...)
{
	int bufferSize = 256;
	while (true) {
		char* buffer = (char*)allocator->Allocate(bufferSize * sizeof(char));
		va_list args;
		va_start(args, format);
		int length = stbsp_vsnprintf(buffer, bufferSize, format, args);
		va_end(args);
		if (length < 0) {
			return { .size = 0, .data = nullptr };
		}
		else if (length < bufferSize) {
			return { .size = (uint64)length, .data = buffer };
		}
		else {
			bufferSize *= 2;
		}
	}
}

template <typename T>
bool32 StringToElementArray(const Array<char>& string, char sep, bool trimElements,
	bool32 (*conversionFunction)(const Array<char>&, T*),
	int maxElements, T* array, int* numElements)
{
	int elementInd = 0;
	Array<char> element = string;
	while (true) {
		Array<char> next;
		ReadElementInSplitString(&element, &next, sep);
		Array<char> trimmed;
		if (trimElements) {
			TrimWhitespace(element, &trimmed);
		}
		else {
			trimmed = element;
		}
		if (!conversionFunction(trimmed, array + elementInd)) {
			LOG_ERROR("String to array failed for %.*s in element %d conversion\n",
				string.size, string.data, elementInd);
			return false;
		}

		if (next.size == 0) {
			break;
		}
		element = next;
		elementInd++;
		if (elementInd >= maxElements) {
			LOG_ERROR("String to array failed in %.*s (too many elements, max %d)\n",
				string.size, string.data, maxElements);
			return false;
		}
	}

	*numElements = elementInd + 1;
	return true;
}

template <uint64 KEYWORD_SIZE, uint64 VALUE_SIZE>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, FixedArray<char, VALUE_SIZE>* outValue)
{
	if (string.size == 0 || string[0] == '\0') {
		return 0;
	}

	int i = 0;

	outKeyword->size = 0;
	while (i < string.size && !IsWhitespace(string[i])) {
		if (outKeyword->size >= KEYWORD_SIZE) {
			LOG_ERROR("Keyword too long %.*s\n", (int)outKeyword->size, outKeyword->data);
			return -1;
		}
		outKeyword->Append(string[i++]);
	}

	if (i < string.size && IsWhitespace(string[i])) {
		i++;
	}

	outValue->size = 0;
	bool bracketValue = false;
	while (i < string.size) {
		if (string[i] == '\n' || string[i] == '\r') {
			// End of inline value
			i++;
			break;
		}
		if (string[i] == '{' && outValue->size == 0) {
			// Start of bracket value, read in separately
			i++;
			bracketValue = true;
			break;
		}
		if (outValue->size >= VALUE_SIZE) {
			LOG_ERROR("Value too long %.*s\n", (int)outValue->size, outValue->data);
			return -1;
		}
		if (outValue->size == 0 && IsWhitespace(string[i])) {
			i++;
			continue;
		}

		outValue->Append(string[i++]);
	}

	if (bracketValue) {
		int bracketDepth = 1;
		bool bracketMatched = false;
		while (i < string.size) {
			if (string[i] == '{') {
				bracketDepth++;
			}
			else if (string[i] == '}') {
				bracketDepth--;
				if (bracketDepth == 0) {
					i++;
					bracketMatched = true;
					break;
				}
			}
			if (outValue->size >= VALUE_SIZE) {
				LOG_ERROR("Value too long %.*s\n", (int)outValue->size, outValue->data);
				return -1;
			}
			if (outValue->size == 0 && IsWhitespace(string[i])) {
				// Gobble starting whitespace
				i++;
				continue;
			}

			outValue->Append(string[i++]);
		}

		if (!bracketMatched) {
			LOG_ERROR("Value bracket unmatched pair\n");
			return -1;
		}
	}

	// Trim trailing whitespace
	while (outValue->size > 0 && IsWhitespace(outValue->data[outValue->size - 1])) {
		outValue->size--;
	}

	while (i < string.size && IsWhitespace(string[i])) {
		i++;
	}

	return i;
}
