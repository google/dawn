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

#ifndef SRC_TINT_UTILS_HASHSET_H_
#define SRC_TINT_UTILS_HASHSET_H_

#include <stddef.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {

/// An unordered set that uses a robin-hood hashing algorithm.
template <typename KEY, size_t N, typename HASH = Hasher<KEY>, typename EQUAL = std::equal_to<KEY>>
class Hashset : public HashmapBase<KEY, void, N, HASH, EQUAL> {
    using Base = HashmapBase<KEY, void, N, HASH, EQUAL>;
    using PutMode = typename Base::PutMode;

  public:
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
    utils::Vector<KEY, N2> Vector() const {
        utils::Vector<KEY, N2> out;
        out.Reserve(this->Count());
        for (auto& value : *this) {
            out.Push(value);
        }
        return out;
    }
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_HASHSET_H_
