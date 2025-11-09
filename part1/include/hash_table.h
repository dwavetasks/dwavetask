#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <sys/types.h>

#include <array>
#include <memory>
#include <string>
#include <tuple>

typedef __uint32_t uint32_t;

template<uint32_t Size>
class HashTable
{
public:
    using KeyType = std::string;
    using ValueType = uint32_t;
    using KeyValuePair = std::tuple<KeyType, ValueType>;
    static constexpr uint32_t ProbingFactor = 1;

    HashTable() : data(std::make_unique<std::array<HashElement, Size>>())
    {
        firstElement.rightElement = &lastElement;
        firstElement.leftElement = nullptr;
        lastElement.rightElement = nullptr;
        lastElement.leftElement = &firstElement;
    }
    ~HashTable() = default;
    HashTable(const HashTable &other) = delete;
    HashTable(HashTable &&other) = delete;
    HashTable &operator=(const HashTable &other) = delete;
    HashTable &operator=(HashTable &&other) = delete;

    bool insert(const KeyType &key, const ValueType &value)
    {
        // Get hash index
        uint32_t index = getHash(key);
        const uint32_t startIndex = index;
        // We dont care if the element is erased or not we just need to find the next free slot
        while (isOccupied(index) && !keysMatch((*data)[index].key, key))
        {
            // Linear probing
            index = (index + ProbingFactor) % Size;
            if (index == startIndex)
            {
                // Table full
                return false;
            }
        }
        (*data)[index].value = value;
        (*data)[index].key = key;

        // If previously erased, reset erased flag
        if ((*data)[index].erased)
        {
            (*data)[index].erased = false;
        }

        // Link this element to the beginning of the list
        // First unlink in case it was already occupied meaning that the keys matched
        unlinkElement(index);
        linkElement(index);

        return true;
    }

    bool remove(const KeyType &key)
    {
        // Get coorect index from probing
        uint32_t index = getIndexFromProbing(key);
        if (index == Size)
        {
            // Key not found
            return false;
        }

        // Unlink element from double linked list
        unlinkElement(index);

        // Set erased as true in order to not break probing chains
        (*data)[index].erased = true;

        // Clear key and value
        (*data)[index].key.clear();
        (*data)[index].value = ValueType{};

        return true;
    }

    std::tuple<bool, ValueType> get(const KeyType &key)
    {
        // Get coorect index from probing
        uint32_t index = getIndexFromProbing(key);
        if (index == Size)
        {
            // Key not found
            return std::make_tuple(false, ValueType{});
        }

        // Update LRU linked list since this element was just accessed
        unlinkElement(index);
        linkElement(index);

        return std::make_tuple(true, (*data)[index].value);
    }

    std::tuple<bool, KeyValuePair> get_last() const
    {
        if (firstElement.rightElement == &lastElement)
        {
            // Table is empty
            return std::make_tuple(false, KeyValuePair{});
        }
        auto keyValuePair = std::make_tuple(firstElement.rightElement->key, firstElement.rightElement->value);
        return std::make_tuple(true, keyValuePair);
    }

    std::tuple<bool, KeyValuePair> get_first() const
    {
        if (lastElement.leftElement == &firstElement)
        {
            // Table is empty
            return std::make_tuple(false, KeyValuePair{});
        }
        auto keyValuePair = std::make_tuple(lastElement.leftElement->key, lastElement.leftElement->value);
        return std::make_tuple(true, keyValuePair);
    }

    uint32_t getHash(const KeyType &key) const
    {
        std::hash<KeyType> hasher;
        return hasher(key) % Size;
    }

private:
    struct HashElement
    {
        HashElement *rightElement = nullptr;
        HashElement *leftElement = nullptr;
        ValueType value{};
        KeyType key{};
        bool erased = false;
    };
    using HashElementPtr = HashElement *;

    // Since we use first and last elements in the double linked list to avoid edge cases,
    // an element is occupied if both left and right pointers are not null
    bool isOccupied(const uint32_t index) const
    {
        return (*data)[index].rightElement != nullptr && (*data)[index].leftElement != nullptr;
    }

    bool keysMatch(const KeyType &key1, const KeyType &key2) const
    {
        return key1 == key2;
    }

    uint32_t getIndexFromProbing(const KeyType &key) const
    {
        uint32_t index = getHash(key);
        const uint32_t startIndex = index;
        // First check if the exists in the hash table
        // If the node we hit from this hash is not occupied or not erased then this key deos not exist in the
        // table
        if (!isOccupied(index) && !(*data)[index].erased)
        {
            return Size; // Indicate not found
        }
        // If the key at this index does not match the key we are looking for, we need to probe linearly
        while (!keysMatch((*data)[index].key, key))
        {
            // Linear probing
            index = (index + ProbingFactor) % Size;
            if (index == startIndex || (!isOccupied(index) && !(*data)[index].erased))
            {
                // We have looped through the entire table or hit an unoccupied slot so key does not exist
                return Size; // Indicate not found
            }
        }
        return index;
    }

    void linkElement(uint32_t index)
    {
        // Link this element to the beginning of the list
        // Left to right connection
        HashElementPtr temp = firstElement.rightElement;
        firstElement.rightElement = &(*data)[index];
        (*data)[index].rightElement = temp;

        // Right to left connection
        temp->leftElement = &(*data)[index];
        (*data)[index].leftElement = &firstElement;
    }

    void unlinkElement(uint32_t index)
    {
        // Element must be occupied to be unlinked
        if (!isOccupied(index))
        {
            return;
        }
        // Unlink the element from the douvble linked list
        // Unlink from left to right
        (*data)[index].leftElement->rightElement = (*data)[index].rightElement;
        // Unlink from right to left
        (*data)[index].rightElement->leftElement = (*data)[index].leftElement;

        // Set null the links of the node to remove so to become unoccupied
        (*data)[index].leftElement = nullptr;
        (*data)[index].rightElement = nullptr;
    }

    // Use if first and last elements to avoid edges cases
    HashElement firstElement{};
    HashElement lastElement{};

    // Hash table data storage as a contiguous array for better cache locality
    // Add this to the heap so to not exceed stack size for large tables
    std::unique_ptr<std::array<HashElement, Size>> data;
};

#endif // HASH_TABLE_H
