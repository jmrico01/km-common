#include "km_container.h"

#include <typeinfo>

static const float32 HASH_TABLE_MAX_SIZE_TO_CAPACITY = 0.7f;

// Very simple string hash ( djb2 hash, source http://www.cse.yorku.ca/~oz/hash.html )
uint32 KeyHash(const HashKey& key)
{
    uint32 hash = 5381;

    for (uint32 i = 0; i < key.s.size; i++) {
        hash = ((hash << 5) + hash) + key.s[i];
    }

    return hash;
}
bool KeyCompare(const HashKey& key1, const HashKey& key2)
{
    if (key1.s.size != key2.s.size) {
        return false;
    }

    for (uint32 i = 0; i < key1.s.size; i++) {
        if (key1.s[i] != key2.s[i]) {
            return false;
        }
    }

    return true;
}

// TODO dumb wrappers until I figure out a better way to do this at compile time
template <typename Allocator>
void* AllocateOrUseDefaultIfNull(Allocator* allocator, uint32 size)
{
    if (allocator == nullptr) {
        DEBUG_ASSERT(typeid(Allocator) == typeid(StandardAllocator));
        return defaultAllocator_.Allocate(size);
    }
    else {
        return allocator->Allocate(size);
    }
}
template <typename Allocator>
void* ReAllocateOrUseDefaultIfNull(Allocator* allocator, void* memory, uint32 size)
{
    if (allocator == nullptr) {
        DEBUG_ASSERT(typeid(Allocator) == typeid(StandardAllocator));
        return defaultAllocator_.ReAllocate(memory, size);
    }
    else {
        return allocator->ReAllocate(memory, size);
    }
}
template <typename Allocator>
void FreeOrUseDefautIfNull(Allocator* allocator, void* memory)
{
    if (allocator == nullptr) {
        DEBUG_ASSERT(typeid(Allocator) == typeid(StandardAllocator));
        defaultAllocator_.Free(memory);
    }
    else {
        allocator->Free(memory);
    }
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray(Allocator* allocator, uint32 capacity)
{
    Initialize(allocator, capacity);
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>::DynamicArray(const Array<T>& array, Allocator* allocator)
: DynamicArray(array.size < DYNAMIC_ARRAY_START_CAPACITY ? DYNAMIC_ARRAY_START_CAPACITY : array.size, allocator)
{
    FromArray(array);
}

template <typename T, typename Allocator>
Array<T> DynamicArray<T, Allocator>::ToArray() const
{
    return *((Array<T>*)this);
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::FromArray(const Array<T>& array)
{
    if (capacity < array.size) {
        // TODO round to nearest power of 2?
        const bool result = UpdateCapacity(array.size);
        if (!result) {
            DEBUG_PANIC("DynamicArray out of memory\n");
        }
    }

    size = array.size;
    for (uint32 i = 0; i < size; i++) {
        data[i] = array.data[i];
    }
}

template <typename T, typename Allocator>
T* DynamicArray<T, Allocator>::Append()
{
    if (size >= capacity) {
        const bool result = UpdateCapacity(capacity * 2);
        if (!result) {
            DEBUG_PANIC("DynamicArray out of memory\n");
            return nullptr;
        }
    }

    // NOTE nope. not doing this anymore
    //new (&data[size]) T();
    return &data[size++];
}

template <typename T, typename Allocator>
T* DynamicArray<T, Allocator>::Append(const T& element)
{
    T* slot = Append();
    *slot = element;
    return slot;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Append(const Array<T>& array)
{
    Append((const Array<const T>)array);
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Append(const Array<const T>& array)
{
    uint32 newSize = size + array.size;
    if (newSize > capacity) {
        // TODO round to nearest power of 2?
        const bool result = UpdateCapacity(newSize);
        if (!result) {
            DEBUG_PANIC("DynamicArray out of memory\n");
        }
    }

    for (uint32 i = 0; i < array.size; i++) {
        data[size + i] = array.data[i];
    }
    size = newSize;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::RemoveLast()
{
    DEBUG_ASSERT(size > 0);
    size--;
}

template <typename T, typename Allocator>
uint32 DynamicArray<T, Allocator>::IndexOf(const T& value)
{
    for (uint32 i = 0; i < size; i++) {
        if (data[i] == value) {
            return i;
        }
    }
    return size;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Clear()
{
    size = 0;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Initialize(Allocator* allocator, uint32 capacity)
{
    size = 0;
    data = (T*)AllocateOrUseDefaultIfNull(allocator, capacity * sizeof(T));
    DEBUG_ASSERT(data != nullptr);

    this->capacity = capacity;
    this->allocator = allocator;
}

template <typename T, typename Allocator>
void DynamicArray<T, Allocator>::Free()
{
    FreeOrUseDefautIfNull(allocator, data);
}

template <typename T, typename Allocator>
inline T& DynamicArray<T, Allocator>::operator[](uint32 index)
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, typename Allocator>
inline const T& DynamicArray<T, Allocator>::operator[](uint32 index) const
{
    ARRAY_BOUNDS_CHECK(index, size);
    return data[index];
}

template <typename T, typename Allocator>
DynamicArray<T, Allocator>& DynamicArray<T, Allocator>::operator=(const DynamicArray<T, Allocator>& other)
{
    FromArray(other.ToArray());
    return *this;
}

template <typename T, typename Allocator>
bool DynamicArray<T, Allocator>::UpdateCapacity(uint32 newCapacity)
{
    DEBUG_ASSERT(capacity != 0);
    DEBUG_ASSERT(newCapacity != 0);
    void* newMemory = ReAllocateOrUseDefaultIfNull(allocator, data, newCapacity * sizeof(T));
    if (newMemory == nullptr) {
        return false;
    }

    capacity = newCapacity;
    data = (T*)newMemory;
    return true;
}

HashKey::HashKey()
{
    s.Clear();
}

HashKey::HashKey(string str)
{
    WriteString(str);
}

HashKey::HashKey(const_string str)
{
    WriteString(str);
}

HashKey::HashKey(const char* str)
{
    WriteString(str);
}

bool HashKey::WriteString(const_string str)
{
    if (str.size > MAX_LENGTH) {
        return false;
    }

    MemCopy(s.data, str.data, str.size * sizeof(char));
    s.size = str.size;
    return true;
}

bool HashKey::WriteString(const char* str)
{
    return WriteString(ToString(str));
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::HashTable(Allocator* allocator, uint32 capacity)
{
    Initialize(allocator, capacity);
}

template <typename V, typename Allocator>
HashTable<V, Allocator>::~HashTable()
{
    for (uint32 i = 0; i < capacity; i++) {
        pairs[i].~KeyValuePair<V>();
    }
    allocator->Free(pairs);
}

template <typename V, typename Allocator>
V* HashTable<V, Allocator>::Add(const HashKey& key)
{
    DEBUG_ASSERT(GetPair(key) == nullptr);

    if (size >= (uint32)((float32)capacity * HASH_TABLE_MAX_SIZE_TO_CAPACITY)) {
        uint32 newCapacity = NextPrime(capacity * 2);
        pairs = (KeyValuePair<V>*)allocator->ReAllocate(pairs, sizeof(KeyValuePair<V>) * newCapacity);
        if (pairs == nullptr) {
            DEBUG_PANIC("not enough memory for HashTable resize (pairs allocation)\n");
        }

        for (uint32 i = 0; i < capacity; i++) {
            // Don't placement new here, probably? Because it'll reset everything...
            // new (&pairs[i]) KeyValuePair<V>();
        }
        KeyValuePair<V>* oldPairs = (KeyValuePair<V>*)allocator->Allocate(sizeof(KeyValuePair<V>) * capacity);
        if (oldPairs == nullptr) {
            DEBUG_PANIC("not enough memory for HashTable resize (oldPairs allocation)\n");
        }
        defer(allocator->Free(oldPairs));
        MemCopy(oldPairs, pairs, sizeof(KeyValuePair<V>) * capacity);
        DEBUG_PANIC("TODO can't resize+rehash yet\n");
        // capacity = newCapacity;
    }

    KeyValuePair<V>* pair = GetFreeSlot(key);
    DEBUG_ASSERT(pair != nullptr);

    pair->key = key;
    size++;

    return &(pair->value);
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Add(const HashKey& key, const V& value)
{
    *(Add(key)) = value;
}

template <typename V, typename Allocator>
V* HashTable<V, Allocator>::GetValue(const HashKey& key)
{
    KeyValuePair<V>* pair = GetPair(key);
    if (pair == nullptr) {
        return nullptr;
    }

    return &pair->value;
}

template <typename V, typename Allocator>
const V* HashTable<V, Allocator>::GetValue(const HashKey& key) const
{
    const KeyValuePair<V>* pair = GetPair(key);
    if (pair == nullptr) {
        return nullptr;
    }

    return &pair->value;
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Clear()
{
    for (uint32 i = 0; i < capacity; i++) {
        pairs[i].key.s.size = 0;
    }
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Initialize(Allocator* allocator, uint32 capacity)
{
    size = 0;
    uint32 sizeBytes = sizeof(KeyValuePair<V>) * capacity;
    pairs = (KeyValuePair<V>*)allocator->Allocate(sizeBytes);
    if (pairs == nullptr) {
        DEBUG_PANIC("ERROR: not enough memory!\n");
    }

    for (uint32 i = 0; i < capacity; i++) {
        pairs[i].key.s.size = 0;
        // NOTE nope. not doing this anymore
        // new (&pairs[i]) KeyValuePair<V>();
    }

    this->capacity = capacity;
    this->allocator = allocator;
}

template <typename V, typename Allocator>
void HashTable<V, Allocator>::Free()
{
    allocator->Free(pairs);

    capacity = 0;
    size = 0;
}

template <typename V, typename Allocator>
HashTable<V, Allocator>& HashTable<V, Allocator>::operator=(const HashTable<V, Allocator>& other)
{
    DEBUG_ASSERT(capacity == other.capacity); // TODO no rehashing, so we do same-capacity only
    size = other.size;
    for (uint32 i = 0; i < capacity; i++) {
        pairs[i] = other.pairs[i];
    }
    return *this;
}

template <typename V, typename Allocator>
KeyValuePair<V>* HashTable<V, Allocator>::GetPair(const HashKey& key) const
{
    uint32 hashInd = KeyHash(key) % capacity;
    for (uint32 i = 0; i < capacity; i++) {
        KeyValuePair<V>* pair = pairs + hashInd + i;
        if (KeyCompare(pair->key, key)) {
            return pair;
        }
        if (pair->key.s.size == 0) {
            return nullptr;
        }
    }

    return nullptr;
}

template <typename V, typename Allocator>
KeyValuePair<V>* HashTable<V, Allocator>::GetFreeSlot(const HashKey& key)
{
    uint32 hashInd = KeyHash(key) % capacity;
    for (uint32 i = 0; i < capacity; i++) {
        KeyValuePair<V>* pair = pairs + hashInd + i;
        if (pair->key.s.size == 0) {
            return pair;
        }
    }

    return nullptr;
}

