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
