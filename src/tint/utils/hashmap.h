// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_UTILS_HASHMAP_H_
#define SRC_TINT_UTILS_HASHMAP_H_

#include <functional>
#include <optional>
#include <utility>

#include "src/tint/utils/hash.h"
#include "src/tint/utils/hashset.h"

namespace tint::utils {

/// An unordered map that uses a robin-hood hashing algorithm.
///
/// Hashmap internally wraps a Hashset for providing a store for key-value pairs.
///
/// @see Hashset
template <typename K,
          typename V,
          size_t N,
          typename HASH = Hasher<K>,
          typename EQUAL = std::equal_to<K>>
class Hashmap {
    /// Entry holds a key and value pair, and is used as the element type of the underlying Hashset.
    /// Entries are compared and hashed using only the #key.
    /// @see Hasher
    /// @see Equality
    struct Entry {
        /// Constructor from a key and value pair
        Entry(K k, V v) : key(std::move(k)), value(std::move(v)) {}

        /// Copy-constructor.
        Entry(const Entry&) = default;

        /// Move-constructor.
        Entry(Entry&&) = default;

        /// Copy-assignment operator
        Entry& operator=(const Entry&) = default;

        /// Move-assignment operator
        Entry& operator=(Entry&&) = default;

        K key;    /// The map entry key
        V value;  /// The map entry value
    };

    /// Hash provider for the underlying Hashset.
    /// Provides hash functions for an Entry or K.
    /// The hash functions only consider the key of an entry.
    struct Hasher {
        /// Calculates a hash from an Entry
        size_t operator()(const Entry& entry) const { return HASH()(entry.key); }
        /// Calculates a hash from a K
        size_t operator()(const K& key) const { return HASH()(key); }
    };

    /// Equality provider for the underlying Hashset.
    /// Provides equality functions for an Entry or K to an Entry.
    /// The equality functions only consider the key for equality.
    struct Equality {
        /// Compares an Entry to an Entry for equality.
        bool operator()(const Entry& a, const Entry& b) const { return EQUAL()(a.key, b.key); }
        /// Compares a K to an Entry for equality.
        bool operator()(const K& a, const Entry& b) const { return EQUAL()(a, b.key); }
    };

    /// The underlying set
    using Set = Hashset<Entry, N, Hasher, Equality>;

  public:
    /// A Key and Value const-reference pair.
    struct KeyValue {
        /// key of a map entry
        const K& key;
        /// value of a map entry
        const V& value;

        /// Equality operator
        /// @param other the other KeyValue
        /// @returns true if the key and value of this KeyValue are equal to other's.
        bool operator==(const KeyValue& other) const {
            return key == other.key && value == other.value;
        }
    };

    /// STL-style alias to KeyValue.
    /// Used by gmock for the `ElementsAre` checks.
    using value_type = KeyValue;

    /// Iterator for the map.
    /// Iterators are invalidated if the map is modified.
    class Iterator {
      public:
        /// @returns the key of the entry pointed to by this iterator
        const K& Key() const { return it->key; }

        /// @returns the value of the entry pointed to by this iterator
        const V& Value() const { return it->value; }

        /// Increments the iterator
        /// @returns this iterator
        Iterator& operator++() {
            ++it;
            return *this;
        }

        /// Equality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is equal to other
        bool operator==(const Iterator& other) const { return it == other.it; }

        /// Inequality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is not equal to other
        bool operator!=(const Iterator& other) const { return it != other.it; }

        /// @returns a pair of key and value for the entry pointed to by this iterator
        KeyValue operator*() const { return {Key(), Value()}; }

      private:
        /// Friend class
        friend class Hashmap;

        /// Underlying iterator type
        using SetIterator = typename Set::Iterator;

        explicit Iterator(SetIterator i) : it(i) {}

        SetIterator it;
    };

    /// Removes all entries from the map.
    void Clear() { set_.Clear(); }

    /// Adds the key-value pair to the map, if the map does not already contain an entry with a key
    /// equal to `key`.
    /// @param key the entry's key to add to the map
    /// @param value the entry's value to add to the map
    /// @returns true if the entry was added to the map, false if there was already an entry in the
    ///          map with a key equal to `key`.
    template <typename KEY, typename VALUE>
    bool Add(KEY&& key, VALUE&& value) {
        return set_.Add(Entry{std::forward<KEY>(key), std::forward<VALUE>(value)});
    }

    /// Adds the key-value pair to the map, replacing any entry with a key equal to `key`.
    /// @param key the entry's key to add to the map
    /// @param value the entry's value to add to the map
    template <typename KEY, typename VALUE>
    void Replace(KEY&& key, VALUE&& value) {
        set_.Replace(Entry{std::forward<KEY>(key), std::forward<VALUE>(value)});
    }

    /// Searches for an entry with the given key value.
    /// @param key the entry's key value to search for.
    /// @returns the value of the entry with the given key, or no value if the entry was not found.
    std::optional<V> Get(const K& key) {
        if (auto* entry = set_.Find(key)) {
            return entry->value;
        }
        return std::nullopt;
    }

    /// Searches for an entry with the given key value, adding and returning the result of
    /// calling `create` if the entry was not found.
    /// @note: Before calling `create`, the map will insert a zero-initialized value for the given
    /// key, which will be replaced with the value returned by `create`. If `create` adds an entry
    /// with `key` to this map, it will be replaced.
    /// @param key the entry's key value to search for.
    /// @param create the create function to call if the map does not contain the key.
    /// @returns the value of the entry.
    template <typename CREATE>
    V& GetOrCreate(const K& key, CREATE&& create) {
        auto res = set_.Add(Entry{key, V{}});
        if (res.action == AddAction::kAdded) {
            // Store the set generation before calling create()
            auto generation = set_.Generation();
            // Call create(), which might modify this map.
            auto value = create();
            // Was this map mutated?
            if (set_.Generation() == generation) {
                // Calling create() did not touch the map. No need to lookup again.
                res.entry->value = std::move(value);
            } else {
                // Calling create() modified the map. Need to insert again.
                res = set_.Replace(Entry{key, std::move(value)});
            }
        }
        return res.entry->value;
    }

    /// Searches for an entry with the given key value, adding and returning a newly created
    /// zero-initialized value if the entry was not found.
    /// @param key the entry's key value to search for.
    /// @returns the value of the entry.
    V& GetOrZero(const K& key) {
        auto res = set_.Add(Entry{key, V{}});
        return res.entry->value;
    }

    /// Searches for an entry with the given key value.
    /// @param key the entry's key value to search for.
    /// @returns the a pointer to the value of the entry with the given key, or nullptr if the entry
    /// was not found.
    /// @warning the pointer must not be used after the map is mutated
    V* Find(const K& key) {
        if (auto* entry = set_.Find(key)) {
            return &entry->value;
        }
        return nullptr;
    }

    /// Searches for an entry with the given key value.
    /// @param key the entry's key value to search for.
    /// @returns the a pointer to the value of the entry with the given key, or nullptr if the entry
    /// was not found.
    /// @warning the pointer must not be used after the map is mutated
    const V* Find(const K& key) const {
        if (auto* entry = set_.Find(key)) {
            return &entry->value;
        }
        return nullptr;
    }

    /// Removes an entry from the set with a key equal to `key`.
    /// @param key the entry key value to remove.
    /// @returns true if an entry was removed.
    bool Remove(const K& key) { return set_.Remove(key); }

    /// Checks whether an entry exists in the map with a key equal to `key`.
    /// @param key the entry key value to search for.
    /// @returns true if the map contains an entry with the given key.
    bool Contains(const K& key) const { return set_.Contains(key); }

    /// Pre-allocates memory so that the map can hold at least `capacity` entries.
    /// @param capacity the new capacity of the map.
    void Reserve(size_t capacity) { set_.Reserve(capacity); }

    /// @returns the number of entries in the map.
    size_t Count() const { return set_.Count(); }

    /// @returns a monotonic counter which is incremented whenever the map is mutated.
    size_t Generation() const { return set_.Generation(); }

    /// @returns true if the map contains no entries.
    bool IsEmpty() const { return set_.IsEmpty(); }

    /// @returns an iterator to the start of the map
    Iterator begin() const { return Iterator{set_.begin()}; }

    /// @returns an iterator to the end of the map
    Iterator end() const { return Iterator{set_.end()}; }

  private:
    Set set_;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_HASHMAP_H_
