// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_UNIQUE_VECTOR_H_
#define SRC_TINT_UTILS_UNIQUE_VECTOR_H_

#include <cstddef>
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/utils/hashset.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {

/// UniqueVector is an ordered container that only contains unique items.
/// Attempting to add a duplicate is a no-op.
template <typename T, size_t N, typename HASH = std::hash<T>, typename EQUAL = std::equal_to<T>>
struct UniqueVector {
    /// STL-friendly alias to T. Used by gmock.
    using value_type = T;

    /// Constructor
    UniqueVector() = default;

    /// Constructor
    /// @param v the vector to construct this UniqueVector with. Duplicate
    /// elements will be removed.
    explicit UniqueVector(std::vector<T>&& v) {
        for (auto& el : v) {
            Add(el);
        }
    }

    /// add appends the item to the end of the vector, if the vector does not
    /// already contain the given item.
    /// @param item the item to append to the end of the vector
    /// @returns true if the item was added, otherwise false.
    bool Add(const T& item) {
        if (set.Add(item)) {
            vector.Push(item);
            return true;
        }
        return false;
    }

    /// @returns true if the vector contains `item`
    /// @param item the item
    bool Contains(const T& item) const { return set.Contains(item); }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    T& operator[](size_t i) { return vector[i]; }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    const T& operator[](size_t i) const { return vector[i]; }

    /// @returns true if the vector is empty
    bool IsEmpty() const { return vector.IsEmpty(); }

    /// @returns the number of items in the vector
    size_t Length() const { return vector.Length(); }

    /// @returns the pointer to the first element in the vector, or nullptr if the vector is empty.
    const T* Data() const { return vector.IsEmpty() ? nullptr : &vector[0]; }

    /// @returns an iterator to the beginning of the vector
    auto begin() const { return vector.begin(); }

    /// @returns an iterator to the end of the vector
    auto end() const { return vector.end(); }

    /// @returns an iterator to the beginning of the reversed vector
    auto rbegin() const { return vector.rbegin(); }

    /// @returns an iterator to the end of the reversed vector
    auto rend() const { return vector.rend(); }

    /// @returns a reference to the internal vector
    operator VectorRef<T>() const { return vector; }

    /// @returns the std::move()'d vector.
    /// @note The UniqueVector must not be used after calling this method
    VectorRef<T> Release() { return std::move(vector); }

    /// Pre-allocates `count` elements in the vector and set
    /// @param count the number of elements to pre-allocate
    void Reserve(size_t count) {
        vector.Reserve(count);
        set.Reserve(count);
    }

    /// Removes the last element from the vector
    /// @returns the popped element
    T Pop() {
        set.Remove(vector.Back());
        return vector.Pop();
    }

  private:
    Vector<T, N> vector;
    Hashset<T, N, HASH, EQUAL> set;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_UNIQUE_VECTOR_H_
