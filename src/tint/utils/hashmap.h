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

#include "src/tint/debug.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/hashmap_base.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {

/// An unordered map that uses a robin-hood hashing algorithm.
template <typename KEY,
          typename VALUE,
          size_t N,
          typename HASH = Hasher<KEY>,
          typename EQUAL = std::equal_to<KEY>>
class Hashmap : public HashmapBase<KEY, VALUE, N, HASH, EQUAL> {
    using Base = HashmapBase<KEY, VALUE, N, HASH, EQUAL>;
    using PutMode = typename Base::PutMode;

  public:
    /// The key type
    using Key = KEY;
    /// The value type
    using Value = VALUE;
    /// The key-value type for a map entry
    using Entry = KeyValue<Key, Value>;

    /// Result of Add()
    using AddResult = typename Base::PutResult;

    /// Adds a value to the map, if the map does not already contain an entry with the key @p key.
    /// @param key the entry key.
    /// @param value the value of the entry to add to the map.
    /// @returns A AddResult describing the result of the add
    template <typename K, typename V>
    AddResult Add(K&& key, V&& value) {
        return this->template Put<PutMode::kAdd>(std::forward<K>(key), std::forward<V>(value));
    }

    /// Adds a new entry to the map, replacing any entry that has a key equal to @p key.
    /// @param key the entry key.
    /// @param value the value of the entry to add to the map.
    /// @returns A AddResult describing the result of the replace
    template <typename K, typename V>
    AddResult Replace(K&& key, V&& value) {
        return this->template Put<PutMode::kReplace>(std::forward<K>(key), std::forward<V>(value));
    }

    /// @param key the key to search for.
    /// @returns the value of the entry that is equal to `value`, or no value if the entry was not
    ///          found.
    std::optional<Value> Get(const Key& key) const {
        if (auto [found, index] = this->IndexOf(key); found) {
            return this->slots_[index].entry->value;
        }
        return std::nullopt;
    }

    /// Searches for an entry with the given key, adding and returning the result of calling
    /// @p create if the entry was not found.
    /// @note: Before calling `create`, the map will insert a zero-initialized value for the given
    /// key, which will be replaced with the value returned by @p create. If @p create adds an entry
    /// with @p key to this map, it will be replaced.
    /// @param key the entry's key value to search for.
    /// @param create the create function to call if the map does not contain the key.
    /// @returns the value of the entry.
    template <typename K, typename CREATE>
    Value& GetOrCreate(K&& key, CREATE&& create) {
        auto res = Add(std::forward<K>(key), Value{});
        if (res.action == MapAction::kAdded) {
            // Store the map generation before calling create()
            auto generation = this->Generation();
            // Call create(), which might modify this map.
            auto value = create();
            // Was this map mutated?
            if (this->Generation() == generation) {
                // Calling create() did not touch the map. No need to lookup again.
                *res.value = std::move(value);
            } else {
                // Calling create() modified the map. Need to insert again.
                res = Replace(key, std::move(value));
            }
        }
        return *res.value;
    }

    /// Searches for an entry with the given key value, adding and returning a newly created
    /// zero-initialized value if the entry was not found.
    /// @param key the entry's key value to search for.
    /// @returns the value of the entry.
    template <typename K>
    Value& GetOrZero(K&& key) {
        auto res = Add(std::forward<K>(key), Value{});
        return *res.value;
    }

    /// @param key the key to search for.
    /// @returns a pointer to the entry that is equal to the given value, or nullptr if the map does
    ///          not contain the given value.
    const Value* Find(const Key& key) const {
        if (auto [found, index] = this->IndexOf(key); found) {
            return &this->slots_[index].entry->value;
        }
        return nullptr;
    }

    /// @param key the key to search for.
    /// @returns a pointer to the entry that is equal to the given value, or nullptr if the map does
    ///          not contain the given value.
    Value* Find(const Key& key) {
        if (auto [found, index] = this->IndexOf(key); found) {
            return &this->slots_[index].entry->value;
        }
        return nullptr;
    }
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_HASHMAP_H_
