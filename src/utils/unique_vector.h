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

#ifndef SRC_UTILS_UNIQUE_VECTOR_H_
#define SRC_UTILS_UNIQUE_VECTOR_H_

#include <unordered_set>
#include <vector>

namespace tint {

/// UniqueVector is an ordered container that only contains unique items.
/// Attempting to add a duplicate is a no-op.
template <typename T>
struct UniqueVector {
  /// The iterator returned by begin() and end()
  using ConstIterator = typename std::vector<T>::const_iterator;

  /// add appends the item to the end of the vector, if the vector does not
  /// already contain the given item.
  /// @param item the item to append to the end of the vector
  void add(const T& item) {
    if (set.count(item) == 0) {
      vector.emplace_back(item);
      set.emplace(item);
    }
  }

  /// @returns the number of items in the vector
  size_t size() const { return vector.size(); }

  /// @returns an iterator to the beginning of the vector
  ConstIterator begin() const { return vector.begin(); }

  /// @returns an iterator to the end of the vector
  ConstIterator end() const { return vector.end(); }

  /// @returns a const reference to the internal vector
  operator const std::vector<T>&() const { return vector; }

 private:
  std::vector<T> vector;
  std::unordered_set<T> set;
};

}  // namespace tint

#endif  //  SRC_UTILS_UNIQUE_VECTOR_H_
