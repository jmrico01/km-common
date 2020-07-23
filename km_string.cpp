#include "km_string.h"

#include <ctype.h>
#include <stb_sprintf.h>
#define UTF8PROC_STATIC
#ifdef KM_UTF8
#include <utf8proc.h>
#endif

#include "km_math.h"

uint32 StringLength(const char* str)
{
    uint32 length = 0;
    while (*(str++) != '\0') {
        length++;
    }

    return length;
}

const_string ToString(const char* cString)
{
    return const_string {
        .size = StringLength(cString),
        .data = (char*)cString
    };
}

string ToNonConstString(const_string constString)
{
    return string {
        .size = constString.size,
        .data = (char*)constString.data
    };
}

#ifdef KM_CPP_STD
string ToString(const std::string& str)
{
    return {
        .size = str.size(),
        .data = (char*)str.c_str()
    };
}
#endif

template <uint32 S>
void InitFromCString(FixedArray<char, S>* string, const char* cString)
{
    uint32 stringLength = StringLength(cString);
    if (stringLength > S) {
        stringLength = S;
    }
    string->array.size = stringLength;
    MemCopy(string->fixedArray, cString, stringLength * sizeof(char));
}

template <typename Allocator>
char* ToCString(const_string str, Allocator* allocator)
{
    char* cString = (char*)allocator->Allocate(str.size + 1);
    if (!cString) {
        return nullptr;
    }
    MemCopy(cString, str.data, str.size * sizeof(char));
    cString[str.size] = '\0';
    return cString;
}

int StringCompare(const_string str1, const_string str2)
{
    uint32 minSize = MinUInt32(str1.size, str2.size);
    for (uint32 i = 0; i < minSize; i++) {
        if (str1[i] < str2[i]) {
            return -1;
        }
        else if (str1[i] > str2[i]) {
            return 1;
        }
    }

    if (str1.size < str2.size) {
        return -1;
    }
    else if (str1.size > str2.size) {
        return 1;
    }
    return 0;
}

bool StringEquals(const_string str1, const_string str2)
{
    if (str1.size != str2.size) {
        return false;
    }

    for (uint32 i = 0; i < str1.size; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }

    return true;
}

void CatStrings(size_t sourceACount, const char* sourceA,
                size_t sourceBCount, const char* sourceB,
                size_t destCount, char* dest)
{
    DEBUG_ASSERT(sourceACount + sourceBCount <= destCount);

    for (size_t i = 0; i < sourceACount; i++) {
        *dest++ = *sourceA++;
    }

    for (size_t i = 0; i < sourceBCount; i++) {
        *dest++ = *sourceB++;
    }

    *dest++ = '\0';
}

void StringCat(const char* str1, const char* str2, char* dest, uint32 destMaxLength)
{
    CatStrings(StringLength(str1), str1, StringLength(str2), str2, destMaxLength, dest);
}

// TODO slow, naive implementation, but perfectly fine for small strings
uint32 SubstringSearch(const_string str, const_string substr)
{
    for (uint32 i = 0; i < str.size; i++) {
        bool match = true;
        for (uint32 j = 0; j < substr.size; j++) {
            uint32 ind = i + j;
            if (ind >= str.size) {
                match = false;
                break;
            }
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }

    return str.size;
}

bool StringContains(const_string str, const_string substr)
{
    return SubstringSearch(str, substr) != str.size;
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

inline bool IsAlphanumeric(char c)
{
    return isalnum(c) != 0;
}

string TrimWhitespace(const_string str)
{
    uint32 start = 0;
    while (start < str.size && IsWhitespace(str[start])) {
        start++;
    }
    uint32 end = str.size;
    while (end > 0 && IsWhitespace(str[end - 1])) {
        end--;
    }

    return string {
        .size = end - start,
        .data = (char*)str.data + start
    };
}

bool StringToIntBase10(const_string str, int* intBase10)
{
    if (str.size == 0) {
        return false;
    }

    bool negative = false;
    *intBase10 = 0;
    for (uint32 i = 0; i < str.size; i++) {
        char c = str[i];
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

bool StringToUInt32Base10(const_string str, uint32* intBase10)
{
    if (str.size == 0) {
        return false;
    }

    *intBase10 = 0;
    for (uint32 i = 0; i < str.size; i++) {
        char c = str[i];
        *intBase10 = (*intBase10) * 10 + (uint32)(c - '0');
    }

    return true;
}

bool StringToFloat32(const_string str, float32* f)
{
    uint32 dotIndex = 0;
    while (dotIndex < str.size && str[dotIndex] != '.') {
        dotIndex++;
    }

    int whole = 0;
    float32 wholeNegative = false;
    if (dotIndex > 0) {
        const_string stringWhole = str.SliceTo(dotIndex);
        if (!StringToIntBase10(stringWhole, &whole)) {
            return false;
        }
        wholeNegative = str[0] == '-';
    }

    *f = (float32)whole;

    int frac = 0;
    if (dotIndex + 1 < str.size) {
        const_string fracString = str.SliceFrom(dotIndex + 1);
        if (!StringToIntBase10(fracString, &frac)) {
            return false;
        }

        frac = wholeNegative ? -frac : frac;
        float32 fractional = (float32)frac;
        for (uint32 i = 0; i < fracString.size; i++) {
            fractional /= 10.0f;
        }
        *f += fractional;
    }
    return true;
}

template <typename Allocator>
void StringSplit(const_string str, char c, DynamicArray<string, Allocator>* outSplit)
{
    outSplit->Clear();

    string s = ToNonConstString(str);
    while (true) {
        uint32 next = s.FindFirst(c);
        if (next == s.size) {
            outSplit->Append(s);
            break;
        }
        uint32 newSize = s.size - next - 1;
        s.size = next;
        outSplit->Append(s);

        s.data += next + 1;
        s.size = newSize;
    }
}

string NextSplitElement(string* str, char separator)
{
    string next = *str;
    for (uint32 i = 0; i < str->size; i++) {
        if ((*str)[i] == separator) {
            next.size = i;
            str->size--;
            break;
        }
    }

    str->size -= next.size;
    str->data += next.size + 1;
    return next;
}

template <typename Allocator>
string StringConcatenate(const_string str1, const_string str2, Allocator* allocator)
{
    string result;
    result.size = str1.size + str2.size;
    result.data = allocator->New<char>(result.size);
    if (result.data == nullptr) {
        return string::empty;
    }
    MemCopy(result.data, str1.data, str1.size);
    MemCopy(result.data + str1.size, str2.data, str2.size);
    return result;
}

bool SizedPrintf(string* str, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int length = stbsp_vsnprintf(str->data, str->size, format, args);
    va_end(args);

    if (length < 0 || length >= (int)str->size) {
        return false;
    }

    str->size = length;
    return true;
}

template <typename Allocator>
string AllocPrintf(Allocator* allocator, const char* format, ...)
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
            return { .size = (uint32)length, .data = buffer };
        }
        else {
            bufferSize *= 2;
        }
    }
}

template <typename Allocator>
char* AllocPrintfDynamicArrayCallback(const char* buffer, void* user, int length)
{
    DynamicArray<char, Allocator>* result = *((DynamicArray<char, Allocator>*)user);
    result->Append(string { .size = length, .data = buffer });
    return buffer;
}

template <typename Allocator>
DynamicArray<char, Allocator> AllocPrintf(const char* format, ...)
{
    DynamicArray<char, Allocator> result;

    char buffer[STB_SPRINTF_MIN];
    va_list args;
    va_start(args, format);
    int length = stbsp_vsprintfcb(AllocPrintfDynamicArrayCallback<Allocator>,
                                  &result, buffer, format, args);
    va_end(args);

    if (length < 0) {
        result.Clear();
    }
    else if (length != result.size) {
        result.Clear();
    }

    return result;
}

template <typename T>
bool StringToElementArray(const_string str, char sep, bool trimElements,
                          bool (*conversionFunction)(const_string, T*),
                          int maxElements, T* array, int* numElements)
{
    int elementInd = 0;
    string s = ToNonConstString(str);
    while (true) {
        string next = NextSplitElement(&s, sep);
        string trimmed;
        if (trimElements) {
            trimmed = TrimWhitespace(next);
        }
        else {
            trimmed = next;
        }
        if (!conversionFunction(trimmed, array + elementInd)) {
            LOG_ERROR("String to array failed for %.*s in element %d conversion\n",
                      (int)str.size, str.data, elementInd);
            return false;
        }

        if (s.size == 0) {
            break;
        }
        elementInd++;
        if (elementInd >= maxElements) {
            LOG_ERROR("String to array failed in %.*s (too many elements, max %d)\n",
                      (int)str.size, str.data, maxElements);
            return false;
        }
    }

    *numElements = elementInd + 1;
    return true;
}

#ifdef KM_UTF8
template <typename Allocator>
bool Utf8ToUppercase(const_string utf8String, DynamicArray<char, Allocator>* outString)
{
    FixedArray<char, 4> utf8Buffer;
    uint64 i = 0;
    while (i < utf8String.size) {
        int32 codePoint;
        utf8proc_ssize_t codePointBytes = utf8proc_iterate((uint8*)&utf8String[i],
                                                           utf8String.size - i, &codePoint);
        if (codePointBytes < 0) {
            LOG_ERROR("Invalid UTF-8 bytes\n");
            return false;
        }
        i += codePointBytes;

        int32 codePointUpper = utf8proc_toupper(codePoint);
        utf8proc_ssize_t codePointUpperBytes = utf8proc_encode_char(codePointUpper,
                                                                    (uint8*)utf8Buffer.data);
        if (codePointUpperBytes == 0) {
            LOG_ERROR("Failed to write UTF-8 codePointUpper\n");
            return false;
        }
        utf8Buffer.size = codePointUpperBytes;
        outString->Append(utf8Buffer.ToArray());
    }

    return true;
}
#endif
