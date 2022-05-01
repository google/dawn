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

namespace tint::utils {

/// UniqueVector is an ordered container that only contains unique items.
/// Attempting to add a duplicate is a no-op.
template <typename T, typename HASH = std::hash<T>, typename EQUAL = std::equal_to<T>>
struct UniqueVector {
    /// The iterator returned by begin() and end()
    using ConstIterator = typename std::vector<T>::const_iterator;
    /// The iterator returned by rbegin() and rend()
    using ConstReverseIterator = typename std::vector<T>::const_reverse_iterator;

    /// Constructor
    UniqueVector() = default;

    /// Constructor
    /// @param v the vector to construct this UniqueVector with. Duplicate
    /// elements will be removed.
    explicit UniqueVector(std::vector<T>&& v) {
        for (auto& el : v) {
            add(el);
        }
    }

    /// add appends the item to the end of the vector, if the vector does not
    /// already contain the given item.
    /// @param item the item to append to the end of the vector
    /// @returns true if the item was added, otherwise false.
    bool add(const T& item) {
        if (set.count(item) == 0) {
            vector.emplace_back(item);
            set.emplace(item);
            return true;
        }
        return false;
    }

    /// @returns true if the vector contains `item`
    /// @param item the item
    bool contains(const T& item) const { return set.count(item); }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    T& operator[](size_t i) { return vector[i]; }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    const T& operator[](size_t i) const { return vector[i]; }

    /// @returns true if the vector is empty
    bool empty() const { return vector.empty(); }

    /// @returns the number of items in the vector
    size_t size() const { return vector.size(); }

    /// @returns an iterator to the beginning of the vector
    ConstIterator begin() const { return vector.begin(); }

    /// @returns an iterator to the end of the vector
    ConstIterator end() const { return vector.end(); }

    /// @returns an iterator to the beginning of the reversed vector
    ConstReverseIterator rbegin() const { return vector.rbegin(); }

    /// @returns an iterator to the end of the reversed vector
    ConstReverseIterator rend() const { return vector.rend(); }

    /// @returns a const reference to the internal vector
    operator const std::vector<T>&() const { return vector; }

    /// Removes the last element from the vector
    /// @returns the popped element
    T pop_back() {
        auto el = std::move(vector.back());
        set.erase(el);
        vector.pop_back();
        return el;
    }

  private:
    std::vector<T> vector;
    std::unordered_set<T, HASH, EQUAL> set;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_UNIQUE_VECTOR_H_
