#include "km_string.h"

#include <cstdarg>
#include <locale>
#undef STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

#include "km_debug.h"
#include "km_math.h"

uint64 StringLength(const char* string)
{
	uint64 length = 0;
	while (*(string++) != '\0') {
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

template <typename Allocator>
char* ToCString(const Array<char>& string, Allocator* allocator)
{
	char* cString = (char*)allocator->Allocate(string.size + 1);
	MemCopy(cString, string.data, string.size * sizeof(char));
	cString[string.size] = '\0';
	return cString;
}

bool StringCompare(const Array<char>& str1, const Array<char>& str2)
{
	if (str1.size != str2.size) {
		return false;
	}

	for (uint64 i = 0; i < str1.size; i++) {
		if (str1[i] != str2[i]) {
			return false;
		}
	}

	return true;
}

bool StringCompare(const Array<char>& str1, const char* str2)
{
	// TODO slow... but it works
	return StringCompare(str1, ToString(str2));
}

bool StringCompare(const char* str1, const char* str2)
{
	// TODO slow... but it works
	return StringCompare(ToString(str1), ToString(str2));
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

inline bool IsNewline(char c)
{
	return c == '\n' || c == '\r';
}

inline bool IsWhitespace(char c)
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

bool StringToIntBase10(const Array<char>& string, int* intBase10)
{
	if (string.size == 0) {
		return false;
	}

	bool negative = false;
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

bool StringToUInt64Base10(const Array<char>& string, uint64* intBase10)
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

bool StringToFloat32(const Array<char>& string, float32* f)
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
bool Utf8ToUppercase(const Array<char>& utf8String, DynamicArray<char, Allocator>* outString)
{
	const uint8 MASK_BIT_8 = 0b10000000;
	const uint8 MASK_BIT_7 = 0b01000000;
	const uint8 MASK_BIT_6 = 0b00100000;
	const uint32 UTF8_MAX_2_BYTE_CHAR = 0x07ff;

	std::locale locale = std::locale("");

	outString->Clear();
	for (uint64 i = 0; i < utf8String.size; i++) {
		uint8 byte1 = (uint8)utf8String[i];
		if ((byte1 & MASK_BIT_8) == 0) {
			outString->Append((char)std::toupper((char)byte1, locale));
		}
		else if ((byte1 & MASK_BIT_7) != 0 && (byte1 & MASK_BIT_6) == 0) {
			if (i + 1 >= utf8String.size) {
				return false;
			}
			uint8 byte2 = (uint8)utf8String[++i];
			if ((byte2 & MASK_BIT_8) == 0 || (byte2 & MASK_BIT_7) != 0) {
				LOG_ERROR("Bad 2-byte character in UTF-8 string\n");
				return false;
			}
			uint16 charUtf16 = (byte2 & 0b00111111) + ((byte1 & 0b00011111) << 6);
			// TODO fix this, crashes on Linux bc locale yada yada
			uint16 charUtf16Upper = std::toupper(charUtf16, locale);
			if (charUtf16Upper > UTF8_MAX_2_BYTE_CHAR) {
				LOG_ERROR("Unsupported UTF-8 character (> 2 bytes) after toupper\n");
				return false;
			}

			uint32 byte1Upper = 0b11000000 + ((charUtf16Upper & 0b11111000000) >> 6);
			uint32 byte2Upper = 0b10000000 +  (charUtf16Upper & 0b00000111111);
			if (byte1Upper > 255) {
				LOG_ERROR("Error forming byte1Upper: %d\n", byte1Upper);
				return false;
			}
			if (byte2Upper > 255) {
				LOG_ERROR("Error forming byte2Upper: %d\n", byte2Upper);
				return false;
			}
			outString->Append((char)byte1Upper);
			outString->Append((char)byte2Upper);
		}
		else {
			LOG_ERROR("Unsupported UTF-8 character (> 2 bytes)\n");
			return false;
		}
	}

	return true;
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
bool StringToElementArray(const Array<char>& string, char sep, bool trimElements,
	bool (*conversionFunction)(const Array<char>&, T*),
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
