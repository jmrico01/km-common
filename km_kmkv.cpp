#include "km_kmkv.h"

#ifdef KM_KMKV_JSON
#include <cJSON.h>
#endif

#include "km_string.h"

template <typename Allocator>
DynamicArray<char, Allocator>* GetKmkvItemStrValue(
	HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
{
	const KmkvItem<Allocator>* itemValuePtr = kmkv.GetValue(itemKey);
	if (itemValuePtr == nullptr) {
		return nullptr;
	}
	if (!itemValuePtr->isString) {
		return nullptr;
	}
	return itemValuePtr->dynamicStringPtr;
}

template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemStrValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
{
	return GetKmkvItemStrValue(const_cast<HashTable<KmkvItem<Allocator>>&>(kmkv), itemKey);
}

template <typename Allocator>
HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(
	HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
{
	const KmkvItem<Allocator>* itemValuePtr = kmkv.GetValue(itemKey);
	if (itemValuePtr == nullptr) {
		return nullptr;
	}
	if (itemValuePtr->isString) {
		return nullptr;
	}
	return itemValuePtr->hashTablePtr;
}

template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemObjValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
{
	return GetKmkvItemObjValue(const_cast<HashTable<KmkvItem<Allocator>>&>(kmkv), itemKey);
}

template <uint64 KEYWORD_SIZE, typename Allocator>
int ReadNextKeywordValue(const Array<char>& string,
	FixedArray<char, KEYWORD_SIZE>* outKeyword, DynamicArray<char, Allocator>* outValue)
{
	if (string.size == 0 || string[0] == '\0') {
		return 0;
	}

	int i = 0;
	outKeyword->Clear();
	while (i < (int)string.size && !IsWhitespace(string[i])) {
		if (outKeyword->size >= KEYWORD_SIZE) {
			LOG_ERROR("Keyword too long %.*s\n", (int)outKeyword->size, outKeyword->data);
			return -1;
		}
		outKeyword->Append(string[i++]);
	}

	if (i < (int)string.size && IsWhitespace(string[i])) {
		i++;
	}

	outValue->Clear();
	bool bracketValue = false;
	while (i < (int)string.size) {
		if (IsNewline(string[i])) {
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
		if (outValue->size == 0 && IsWhitespace(string[i])) {
			i++;
			continue;
		}

		outValue->Append(string[i++]);
	}

	if (bracketValue) {
		int bracketDepth = 1;
		bool prevNewline = false;
		bool bracketMatched = false;
		while (i < (int)string.size) {
			if (string[i] == '{' && (i + 1 < (int)string.size) && IsNewline(string[i + 1])) {
				bracketDepth++;
			}
			else if (string[i] == '}' && prevNewline) {
				bracketDepth--;
				if (bracketDepth == 0) {
					i++;
					bracketMatched = true;
					break;
				}
			}
			if (IsNewline(string[i])) {
				prevNewline = true;
			}
			else if (prevNewline && !IsWhitespace(string[i])) {
				prevNewline = false;
			}
			if (outValue->size == 0 && IsWhitespace(string[i])) {
				// Gobble starting whitespace
				i++;
				continue;
			}

			outValue->Append(string[i++]);
		}

		if (!bracketMatched) {
			LOG_ERROR("Value bracket unmatched pair for keyword %.*s\n",
				(int)outKeyword->size, outKeyword->data);
			return -1;
		}
	}

	// Trim trailing whitespace
	while (outValue->size > 0 && IsWhitespace((*outValue)[outValue->size - 1])) {
		outValue->RemoveLast();
	}

	while (i < (int)string.size && IsWhitespace(string[i])) {
		i++;
	}

	return i;
}

template <typename Allocator>
internal bool LoadKmkvRecursive(Array<char> string, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv)
{
	const uint64 KEYWORD_MAX_LENGTH = 32;
	FixedArray<char, KEYWORD_MAX_LENGTH> keyword;
	DynamicArray<char, Allocator> valueBuffer(KILOBYTES(64));
	KmkvItem<Allocator> kmkvValueItem;
	while (true) {
		int read = ReadNextKeywordValue(string, &keyword, &valueBuffer);
		if (read < 0) {
			LOG_ERROR("kmkv file keyword/value error\n");
			return false;
		}
		else if (read == 0) {
			break;
		}
		string.size -= read;
		string.data += read;

		kmkvValueItem.keywordTag.Clear();
		bool keywordHasTag = false;
		uint64 keywordTagInd = 0;
		while (keywordTagInd < keyword.size) {
			if (keyword[keywordTagInd] == '{') {
				keywordHasTag = true;
				break;
			}
			keywordTagInd++;
		}
		if (keywordHasTag) {
			keywordTagInd++;
			bool bracketMatched = false;
			while (keywordTagInd < keyword.size) {
				if (keyword[keywordTagInd] == '}') {
					bracketMatched = true;
					break;
				}
				kmkvValueItem.keywordTag.Append(keyword[keywordTagInd++]);
			}
			if (!bracketMatched) {
				LOG_ERROR("kmkv keyword tag unmatched bracket\n");
				return false;
			}
			if (bracketMatched && keywordTagInd != keyword.size - 1) {
				LOG_ERROR("found characters after kmkv keyword tag bracket, keyword %.*s\n",
					(int)keyword.size, keyword.data);
				return false;
			}
		}
		Array<char> keywordArray = keyword.ToArray();
		if (keywordHasTag) {
			keywordArray = keywordArray.SliceTo(keywordArray.size - kmkvValueItem.keywordTag.size - 2);
		}
		if (outKmkv->GetValue(keywordArray)) {
			LOG_ERROR("kmkv duplicate keyword: %.*s\n", (int)keywordArray.size, keywordArray.data);
			return false;
		}

		if (StringEquals(kmkvValueItem.keywordTag.ToArray(), ToString("kmkv"))) {
			kmkvValueItem.isString = false;
			// "placement new" - allocate with custom allocator, but still call constructor
			// Also, oh my god... that "template" keyword... C++ SUCKS
			kmkvValueItem.hashTablePtr = allocator->template New<HashTable<KmkvItem<Allocator>, Allocator>>();
			if (kmkvValueItem.hashTablePtr == nullptr) {
				return false;
			}
			new (kmkvValueItem.hashTablePtr) HashTable<KmkvItem<Allocator>>();
			if (!LoadKmkvRecursive(valueBuffer.ToArray(), allocator, kmkvValueItem.hashTablePtr)) {
				return false;
			}
		}
		else {
			kmkvValueItem.isString = true;
			// "placement new" - allocate with custom allocator, but still call constructor
			// Also, oh my god... that "template" keyword... C++ SUCKS
			kmkvValueItem.dynamicStringPtr = allocator->template New<DynamicArray<char>>();
			if (kmkvValueItem.dynamicStringPtr == nullptr) {
				return false;
			}
			new (kmkvValueItem.dynamicStringPtr) DynamicArray<char>(valueBuffer.ToArray());
		}
		outKmkv->Add(keywordArray, kmkvValueItem);
	}

	return true;
}

template <typename Allocator>
bool LoadKmkv(const Array<char>& filePath, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv)
{
	Array<uint8> kmkvFile = LoadEntireFile(filePath, allocator);
	if (kmkvFile.data == nullptr) {
		LOG_ERROR("Failed to load file %.*s\n", (int)filePath.size, filePath.data);
		return false;
	}
	defer(FreeFile(kmkvFile, allocator));

	Array<char> fileString;
	fileString.size = kmkvFile.size;
	fileString.data = (char*)kmkvFile.data;

	outKmkv->Clear();
	return LoadKmkvRecursive(fileString, allocator, outKmkv);
}

template <typename Allocator>
void FreeKmkv(const HashTable<KmkvItem<Allocator>>& kmkv)
{
	// TODO implement
}

template <typename Allocator>
internal bool KmkvToStringRecursive(const HashTable<KmkvItem<Allocator>>& kmkv, int indentSpaces,
	DynamicArray<char, Allocator>* outString)
{
	for (uint64 i = 0; i < kmkv.capacity; i++) {
		const HashKey& key = kmkv.pairs[i].key;
		if (key.string.size == 0) {
			continue;
		}

		for (int i = 0; i < indentSpaces; i++) outString->Append(' ');
		outString->Append(key.string.ToArray());
		const KmkvItem<Allocator>& item = kmkv.pairs[i].value;
		if (item.isString) {
			if (item.keywordTag.size > 0) {
				outString->Append('{');
				outString->Append(item.keywordTag.ToArray());
				outString->Append('}');
			}
			outString->Append(' ');

			bool inlineValue = item.dynamicStringPtr->IndexOf('\n') == item.dynamicStringPtr->size;
			if (!inlineValue) {
				outString->Append('{');
				outString->Append('\n');
			}
			outString->Append(item.dynamicStringPtr->ToArray());
			if (!inlineValue) {
				outString->Append('\n');
				outString->Append('}');
			}
		}
		else {
			outString->Append(ToString("{kmkv} {\n"));
			if (!KmkvToStringRecursive(*item.hashTablePtr, indentSpaces + 4, outString)) {
				LOG_ERROR("Failed to convert nested kmkv to string, key %.*s\n",
					(int)key.string.size, key.string.data);
			}
			outString->Append('}');
		}

		outString->Append('\n');
	}

	return true;
}

template <typename Allocator>
bool KmkvToString(const HashTable<KmkvItem<Allocator>>& kmkv,
	DynamicArray<char, Allocator>* outString)
{
	return KmkvToStringRecursive(kmkv, 0, outString);
}

#ifdef KM_KMKV_JSON
template <typename Allocator>
void AddAndMaybeEscapeJson(const Array<char>& string, DynamicArray<char, Allocator>* outJson)
{
	for (uint64 i = 0; i < string.size; i++) {
		switch (string[i]) {
		case '\b': {
			outJson->Append('\\');
			outJson->Append('b');
		} break;
		case '\f': {
			outJson->Append('\\');
			outJson->Append('f');
		} break;
		case '\n': {
			outJson->Append('\\');
			outJson->Append('n');
		} break;
		case '\r': {
			outJson->Append('\\');
			outJson->Append('r');
		} break;
		case '\t': {
			outJson->Append('\\');
			outJson->Append('t');
		} break;
		case '\"': {
			outJson->Append('\\');
			outJson->Append('"');
		} break;
		case '\\': {
			outJson->Append('\\');
			outJson->Append('\\');
		} break;
		default: {
			outJson->Append(string[i]);
		} break;
		}
	}
}

template <typename Allocator>
internal bool KmkvToJsonRecursive(const HashTable<KmkvItem<Allocator>>& kmkv,
	DynamicArray<char, Allocator>* outJson)
{
	for (uint64 i = 0; i < kmkv.capacity; i++) {
		const HashKey& key = kmkv.pairs[i].key;
		if (key.string.size == 0) {
			continue;
		}

		outJson->Append('"');
		outJson->Append(key.string.ToArray());
		outJson->Append('"');
		outJson->Append(':');
		const KmkvItem<Allocator>& item = kmkv.pairs[i].value;
		if (item.isString) {
			if (StringEquals(item.keywordTag.ToArray(), ToString("array"))) {
				outJson->Append('[');
				Array<char> arrayString = item.dynamicStringPtr->ToArray();
				bool atLeastOne = arrayString.size > 0;
				while (arrayString.size > 0) {
					Array<char> split = NextSplitElement(&arrayString, ',');
					split = TrimWhitespace(split);
					outJson->Append('"');
					AddAndMaybeEscapeJson(split, outJson);
					outJson->Append('"');
					outJson->Append(',');
				}
				if (atLeastOne) {
					outJson->RemoveLast();
				}
				outJson->Append(']');
			}
			else {
				outJson->Append('"');
				AddAndMaybeEscapeJson(item.dynamicStringPtr->ToArray(), outJson);
				outJson->Append('"');
			}
		}
		else {
			outJson->Append('{');
			if (!KmkvToJsonRecursive(*item.hashTablePtr, outJson)) {
				return false;
			}
			outJson->Append('}');
		}

		outJson->Append(',');
	}

	if ((*outJson)[outJson->size - 1] == ',') {
		outJson->RemoveLast();
	}

	return true;
}

template <typename Allocator>
bool KmkvToJson(const HashTable<KmkvItem<Allocator>>& kmkv, DynamicArray<char, Allocator>* outJson)
{
	outJson->Append('{');
	if (!KmkvToJsonRecursive(kmkv, outJson)) {
		return false;
	}
	outJson->Append('}');

	return true;
}

template <typename Allocator>
internal bool JsonToKmkvRecursive(const cJSON* json, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv)
{
	const cJSON* child = json->child;
	while (child != NULL && child->string != NULL) {
		KmkvItem<Allocator>* item = outKmkv->Add(child->string);
		if (cJSON_IsObject(child)) {
			item->isString = false;
			item->hashTablePtr = allocator->template New<HashTable<KmkvItem<Allocator>, Allocator>>();
			DEBUG_ASSERT(item->hashTablePtr != nullptr);
			new (item->hashTablePtr) HashTable<KmkvItem<Allocator>, Allocator>();
			if (!JsonToKmkvRecursive(child, allocator, item->hashTablePtr)) {
				LOG_ERROR("Failed to parse child JSON object, key %s\n", child->string);
				return false;
			}
		}
		else if (cJSON_IsString(child)) {
			item->isString = true;
			item->dynamicStringPtr = allocator->template New<DynamicArray<char, Allocator>>();
			DEBUG_ASSERT(item->dynamicStringPtr != nullptr);
			new (item->dynamicStringPtr) DynamicArray<char, Allocator>(ToString(child->valuestring));
		}
		else if (cJSON_IsArray(child)) {
			item->isString = true;
			item->dynamicStringPtr = allocator->template New<DynamicArray<char, Allocator>>();
			DEBUG_ASSERT(item->dynamicStringPtr != nullptr);
			new (item->dynamicStringPtr) DynamicArray<char, Allocator>();

			const cJSON* arrayItem;
			cJSON_ArrayForEach(arrayItem, child) {
				if (!cJSON_IsString(arrayItem)) {
					LOG_ERROR("JSON array item not a string, key %s\n", child->string);
					return false;
				}
				item->dynamicStringPtr->Append(ToString(arrayItem->valuestring));
				item->dynamicStringPtr->Append(',');
			}
			if (cJSON_GetArraySize(child) > 0) {
				item->dynamicStringPtr->RemoveLast();
			}
			item->keywordTag.Append(ToString("array"));
		}
		else {
			LOG_ERROR("Unhandled JSON type: %d\n", child->type);
			return false;
		}

		child = child->next;
	}

	return true;
}

template <typename Allocator>
bool JsonToKmkv(const Array<char>& jsonString, Allocator* allocator,
	HashTable<KmkvItem<Allocator>>* outKmkv)
{
	const char* jsonCString = ToCString(jsonString, allocator);
	cJSON* json = cJSON_Parse(jsonCString);
	if (json == NULL) {
		const char *errorPtr = cJSON_GetErrorPtr();
		if (errorPtr != NULL) {
			LOG_ERROR("Error parsing JSON before: %s\n", errorPtr);
		}
		return false;
	}
	defer(cJSON_Delete(json));

	if (!cJSON_IsObject(json)) {
		LOG_ERROR("Top-level json not object type: %s\n", jsonCString);
		return false;
	}
	return JsonToKmkvRecursive(json, allocator, outKmkv);
}
#endif
