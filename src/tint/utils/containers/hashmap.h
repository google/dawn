// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_HASHMAP_H_
#define SRC_TINT_UTILS_CONTAINERS_HASHMAP_H_

#include <functional>
#include <optional>
#include <utility>

#include "src/tint/utils/containers/hashmap_base.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"

namespace tint {

/// An unordered map that uses a robin-hood hashing algorithm.
template <typename KEY,
          typename VALUE,
          size_t N,
          typename HASH = Hasher<KEY>,
          typename EQUAL = EqualTo<KEY>>
class Hashmap : public HashmapBase<KEY, VALUE, N, HASH, EQUAL> {
    using Base = HashmapBase<KEY, VALUE, N, HASH, EQUAL>;
    using PutMode = typename Base::PutMode;

    template <typename T>
    using ReferenceKeyType = traits::CharArrayToCharPtr<std::remove_reference_t<T>>;

  public:
    /// The key type
    using Key = KEY;
    /// The value type
    using Value = VALUE;
    /// The key-value type for a map entry
    using Entry = KeyValue<Key, Value>;

    /// Result of Add()
    using AddResult = typename Base::PutResult;

    /// Reference is returned by Hashmap::Find(), and performs dynamic Hashmap lookups.
    /// The value returned by the Reference reflects the current state of the Hashmap, and so the
    /// referenced value may change, or transition between valid or invalid based on the current
    /// state of the Hashmap.
    template <bool IS_CONST, typename K>
    class ReferenceT {
        /// `const Value` if IS_CONST, or `Value` if !IS_CONST
        using T = std::conditional_t<IS_CONST, const Value, Value>;

        /// `const Hashmap` if IS_CONST, or `Hashmap` if !IS_CONST
        using Map = std::conditional_t<IS_CONST, const Hashmap, Hashmap>;

      public:
        /// @returns true if the reference is valid.
        operator bool() const { return Get() != nullptr; }

        /// @returns the pointer to the Value, or nullptr if the reference is invalid.
        operator T*() const { return Get(); }

        /// @returns the pointer to the Value
        /// @warning if the Hashmap does not contain a value for the reference, then this will
        /// trigger a TINT_ASSERT, or invalid pointer dereference.
        T* operator->() const {
            auto* hashmap_reference_lookup = Get();
            TINT_ASSERT(hashmap_reference_lookup != nullptr);
            return hashmap_reference_lookup;
        }

        /// @returns the pointer to the Value, or nullptr if the reference is invalid.
        T* Get() const {
            auto generation = map_.Generation();
            if (generation_ != generation) {
                cached_ = map_.Lookup(key_);
                generation_ = generation;
            }
            return cached_;
        }

      private:
        friend Hashmap;

        /// Constructor
        template <typename K_ARG>
        ReferenceT(Map& map, K_ARG&& key)
            : map_(map),
              key_(std::forward<K_ARG>(key)),
              cached_(nullptr),
              generation_(map.Generation() - 1) {}

        /// Constructor
        template <typename K_ARG>
        ReferenceT(Map& map, K_ARG&& key, T* value)
            : map_(map),
              key_(std::forward<K_ARG>(key)),
              cached_(value),
              generation_(map.Generation()) {}

        Map& map_;
        const K key_;
        mutable T* cached_ = nullptr;
        mutable size_t generation_ = 0;
    };

    /// A mutable reference returned by Find()
    template <typename K>
    using Reference = ReferenceT</*IS_CONST*/ false, K>;

    /// An immutable reference returned by Find()
    template <typename K>
    using ConstReference = ReferenceT</*IS_CONST*/ true, K>;

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
    template <typename K>
    std::optional<Value> Get(K&& key) const {
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
    auto GetOrZero(K&& key) {
        auto res = Add(std::forward<K>(key), Value{});
        return Reference<ReferenceKeyType<K>>(*this, key, res.value);
    }

    /// @param key the key to search for.
    /// @returns a reference to the entry that is equal to the given value.
    template <typename K>
    auto Find(K&& key) {
        return Reference<ReferenceKeyType<K>>(*this, std::forward<K>(key));
    }

    /// @param key the key to search for.
    /// @returns a reference to the entry that is equal to the given value.
    template <typename K>
    auto Find(K&& key) const {
        return ConstReference<ReferenceKeyType<K>>(*this, std::forward<K>(key));
    }

    /// @returns the keys of the map as a vector.
    /// @note the order of the returned vector is non-deterministic between compilers.
    template <size_t N2 = N>
    Vector<Key, N2> Keys() const {
        Vector<Key, N2> out;
        out.Reserve(this->Count());
        for (auto it : *this) {
            out.Push(it.key);
        }
        return out;
    }

    /// @returns the values of the map as a vector
    /// @note the order of the returned vector is non-deterministic between compilers.
    template <size_t N2 = N>
    Vector<Value, N2> Values() const {
        Vector<Value, N2> out;
        out.Reserve(this->Count());
        for (auto it : *this) {
            out.Push(it.value);
        }
        return out;
    }

    /// Equality operator
    /// @param other the other Hashmap to compare this Hashmap to
    /// @returns true if this Hashmap has the same key and value pairs as @p other
    template <typename K, typename V, size_t N2>
    bool operator==(const Hashmap<K, V, N2>& other) const {
        if (this->Count() != other.Count()) {
            return false;
        }
        for (auto it : *this) {
            auto other_val = other.Find(it.key);
            if (!other_val || it.value != *other_val) {
                return false;
            }
        }
        return true;
    }

    /// Inequality operator
    /// @param other the other Hashmap to compare this Hashmap to
    /// @returns false if this Hashmap has the same key and value pairs as @p other
    template <typename K, typename V, size_t N2>
    bool operator!=(const Hashmap<K, V, N2>& other) const {
        return !(*this == other);
    }

  private:
    template <typename K>
    Value* Lookup(K&& key) {
        if (auto [found, index] = this->IndexOf(key); found) {
            return &this->slots_[index].entry->value;
        }
        return nullptr;
    }

    template <typename K>
    const Value* Lookup(K&& key) const {
        if (auto [found, index] = this->IndexOf(key); found) {
            return &this->slots_[index].entry->value;
        }
        return nullptr;
    }
};

/// Hasher specialization for Hashmap
template <typename K, typename V, size_t N, typename HASH, typename EQUAL>
struct Hasher<Hashmap<K, V, N, HASH, EQUAL>> {
    /// @param map the Hashmap to hash
    /// @returns a hash of the map
    size_t operator()(const Hashmap<K, V, N, HASH, EQUAL>& map) const {
        auto hash = Hash(map.Count());
        for (auto it : map) {
            // Use an XOR to ensure that the non-deterministic ordering of the map still produces
            // the same hash value for the same entries.
            hash ^= Hash(it.key, it.value);
        }
        return hash;
    }
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_HASHMAP_H_
