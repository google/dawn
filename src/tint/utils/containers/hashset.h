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

#ifndef SRC_TINT_UTILS_CONTAINERS_HASHSET_H_
#define SRC_TINT_UTILS_CONTAINERS_HASHSET_H_

#include <stddef.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"

namespace tint {

/// An unordered set that uses a robin-hood hashing algorithm.
template <typename KEY, size_t N, typename HASH = Hasher<KEY>, typename EQUAL = std::equal_to<KEY>>
class Hashset : public HashmapBase<KEY, void, N, HASH, EQUAL> {
    using Base = HashmapBase<KEY, void, N, HASH, EQUAL>;
    using PutMode = typename Base::PutMode;

  public:
    using Base::Base;

    /// Constructor with initializer list of items
    /// @param items the items to place into the set
    Hashset(std::initializer_list<KEY> items) {
        this->Reserve(items.size());
        for (auto item : items) {
            this->Add(item);
        }
    }

    /// Adds a value to the set, if the set does not already contain an entry equal to `value`.
    /// @param value the value to add to the set.
    /// @returns true if the value was added, false if there was an existing value in the set.
    template <typename V>
    bool Add(V&& value) {
        struct NoValue {};
        return this->template Put<PutMode::kAdd>(std::forward<V>(value), NoValue{});
    }

    /// @returns the set entries of the map as a vector
    /// @note the order of the returned vector is non-deterministic between compilers.
    template <size_t N2 = N>
    tint::Vector<KEY, N2> Vector() const {
        tint::Vector<KEY, N2> out;
        out.Reserve(this->Count());
        for (auto& value : *this) {
            out.Push(value);
        }
        return out;
    }

    /// @returns true if the predicate function returns true for any of the elements of the set
    /// @param pred a function-like with the signature `bool(T)`
    template <typename PREDICATE>
    bool Any(PREDICATE&& pred) const {
        for (const auto& it : *this) {
            if (pred(it)) {
                return true;
            }
        }
        return false;
    }

    /// @returns false if the predicate function returns false for any of the elements of the set
    /// @param pred a function-like with the signature `bool(T)`
    template <typename PREDICATE>
    bool All(PREDICATE&& pred) const {
        for (const auto& it : *this) {
            if (!pred(it)) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_HASHSET_H_
