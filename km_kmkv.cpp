#include "km_kmkv.h"

#include "km_string.h"

template <typename Allocator>
const DynamicArray<char, Allocator>* GetKmkvItemStrValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
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
const HashTable<KmkvItem<Allocator>>* GetKmkvItemObjValue(
	const HashTable<KmkvItem<Allocator>>& kmkv, const HashKey& itemKey)
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

template <uint64 KEYWORD_SIZE, typename Allocator>
internal int ReadNextKeywordValue(const Array<char>& string,
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
			fprintf(stderr, "kmkv file keyword/value error\n");
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
				fprintf(stderr, "kmkv keyword tag unmatched bracket\n");
				return false;
			}
			if (bracketMatched && keywordTagInd != keyword.size - 1) {
				fprintf(stderr, "found characters after kmkv keyword tag bracket, keyword %.*s\n",
					(int)keyword.size, keyword.data);
				return false;
			}
		}
		if (StringCompare(kmkvValueItem.keywordTag.ToArray(), "kmkv")) {
			kmkvValueItem.isString = false;
			// "placement new" - allocate with custom allocator, but still call constructor
			// Also, oh my god... C++ SUCKS
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
			// Also, oh my god... C++ SUCKS
			kmkvValueItem.dynamicStringPtr = allocator->template New<DynamicArray<char>>();
			if (kmkvValueItem.dynamicStringPtr == nullptr) {
				return false;
			}
			new (kmkvValueItem.dynamicStringPtr) DynamicArray<char>(valueBuffer.ToArray());
		}
		Array<char> keywordArray = keyword.ToArray();
		if (keywordHasTag) {
			keywordArray = keywordArray.SliceTo(keywordArray.size - 2 - kmkvValueItem.keywordTag.size);
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
		fprintf(stderr, "Failed to load file %.*s\n", (int)filePath.size, filePath.data);
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
			bool isArray = StringCompare(item.keywordTag.ToArray(), "array");
			if (!isArray) {
				outJson->Append('"');
			}
			const DynamicArray<char, Allocator>& itemString = *item.dynamicStringPtr;
			for (uint64 j = 0; j < itemString.size; j++) {
				if (isArray) {
					outJson->Append(itemString[j]);
					continue;
				}

				switch (itemString[j]) {
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
					outJson->Append(itemString[j]);
				} break;
				}
			}
			if (!isArray) {
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
	outJson->Clear();

	outJson->Append('{');
	if (!KmkvToJsonRecursive(kmkv, outJson)) {
		return false;
	}
	outJson->Append('}');

	return true;
}
