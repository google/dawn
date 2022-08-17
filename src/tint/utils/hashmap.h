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
          typename HASH = std::hash<K>,
          typename EQUAL = std::equal_to<K>>
class Hashmap {
    /// LazyCreator is a transient structure used to late-build the Entry::value, when inserted into
    /// the underlying Hashset.
    ///
    /// LazyCreator holds a #key, and a #create function used to build the final Entry::value.
    /// The #create function must be of the signature `V()`.
    ///
    /// LazyCreator can be compared to Entry and hashed, allowing them to be passed to
    /// Hashset::Insert(). If the set does not contain an existing entry with #key,
    /// Hashset::Insert() will construct a new Entry passing the rvalue LazyCreator as the
    /// constructor argument, which in turn calls the #create function to generate the entry value.
    ///
    /// @see Entry
    /// @see Hasher
    /// @see Equality
    template <typename CREATE>
    struct LazyCreator {
        /// The key of the entry to insert into the map
        const K& key;
        /// The value creation function
        CREATE create;
    };

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

        /// Constructor from a LazyCreator.
        /// The constructor invokes the LazyCreator::create function to build the #value.
        /// @see LazyCreator
        template <typename CREATE>
        Entry(const LazyCreator<CREATE>& creator)  // NOLINT(runtime/explicit)
            : key(creator.key), value(creator.create()) {}

        /// Assignment operator from a LazyCreator.
        /// The assignment invokes the LazyCreator::create function to build the #value.
        /// @see LazyCreator
        template <typename CREATE>
        Entry& operator=(LazyCreator<CREATE>&& creator) {
            key = std::move(creator.key);
            value = creator.create();
            return *this;
        }

        /// Copy-assignment operator
        Entry& operator=(const Entry&) = default;

        /// Move-assignment operator
        Entry& operator=(Entry&&) = default;

        K key;    /// The map entry key
        V value;  /// The map entry value
    };

    /// Hash provider for the underlying Hashset.
    /// Provides hash functions for an Entry, K or LazyCreator.
    /// The hash functions only consider the key of an entry.
    struct Hasher {
        /// Calculates a hash from an Entry
        size_t operator()(const Entry& entry) const { return HASH()(entry.key); }
        /// Calculates a hash from a K
        size_t operator()(const K& key) const { return HASH()(key); }
        /// Calculates a hash from a LazyCreator
        template <typename CREATE>
        size_t operator()(const LazyCreator<CREATE>& lc) const {
            return HASH()(lc.key);
        }
    };

    /// Equality provider for the underlying Hashset.
    /// Provides equality functions for an Entry, K or LazyCreator to an Entry.
    /// The equality functions only consider the key for equality.
    struct Equality {
        /// Compares an Entry to an Entry for equality.
        bool operator()(const Entry& a, const Entry& b) const { return EQUAL()(a.key, b.key); }
        /// Compares a K to an Entry for equality.
        bool operator()(const K& a, const Entry& b) const { return EQUAL()(a, b.key); }
        /// Compares a LazyCreator to an Entry for equality.
        template <typename CREATE>
        bool operator()(const LazyCreator<CREATE>& lc, const Entry& b) const {
            return EQUAL()(lc.key, b.key);
        }
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

    /// Iterator for the map
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
    /// @param key the entry's key value to search for.
    /// @param create the create function to call if the map does not contain the key.
    /// @returns the value of the entry.
    template <typename CREATE>
    V& GetOrCreate(const K& key, CREATE&& create) {
        LazyCreator<CREATE> lc{key, std::forward<CREATE>(create)};
        auto res = set_.Add(std::move(lc));
        return res.entry->value;
    }

    /// Searches for an entry with the given key value, adding and returning a newly created
    /// zero-initialized value if the entry was not found.
    /// @param key the entry's key value to search for.
    /// @returns the value of the entry.
    V& GetOrZero(const K& key) {
        auto zero = [] { return V{}; };
        LazyCreator<decltype(zero)> lc{key, zero};
        auto res = set_.Add(std::move(lc));
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
