// Copyright 2021 The Tint Authors
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

#ifndef SRC_TINT_UTILS_HASH_H_
#define SRC_TINT_UTILS_HASH_H_

#include <stdint.h>
#include <cstdio>
#include <functional>
#include <utility>
#include <vector>

namespace tint::utils {
namespace detail {

/// Helper for obtaining a seed bias value for HashCombine with a bit-width
/// dependent on the size of size_t.
template <int SIZE_OF_SIZE_T>
struct HashCombineOffset {};

/// Specialization of HashCombineOffset for size_t == 4.
template <>
struct HashCombineOffset<4> {
  /// @returns the seed bias value for HashCombine()
  static constexpr inline uint32_t value() { return 0x7f4a7c16; }
};

/// Specialization of HashCombineOffset for size_t == 8.
template <>
struct HashCombineOffset<8> {
  /// @returns the seed bias value for HashCombine()
  static constexpr inline uint64_t value() { return 0x9e3779b97f4a7c16; }
};

}  // namespace detail

/// HashCombine "hashes" together an existing hash and hashable values.
template <typename T>
void HashCombine(size_t* hash, const T& value) {
  constexpr size_t offset = detail::HashCombineOffset<sizeof(size_t)>::value();
  *hash ^= std::hash<T>()(value) + offset + (*hash << 6) + (*hash >> 2);
}

/// HashCombine "hashes" together an existing hash and hashable values.
template <typename T>
void HashCombine(size_t* hash, const std::vector<T>& vector) {
  HashCombine(hash, vector.size());
  for (auto& el : vector) {
    HashCombine(hash, el);
  }
}

/// HashCombine "hashes" together an existing hash and hashable values.
template <typename T, typename... ARGS>
void HashCombine(size_t* hash, const T& value, const ARGS&... args) {
  HashCombine(hash, value);
  HashCombine(hash, args...);
}

/// @returns a hash of the combined arguments. The returned hash is dependent on
/// the order of the arguments.
template <typename... ARGS>
size_t Hash(const ARGS&... args) {
  size_t hash = 102931;  // seed with an arbitrary prime
  HashCombine(&hash, args...);
  return hash;
}

/// Wrapper for a hashable type enabling the wrapped value to be used as a key
/// for an unordered_map or unordered_set.
template <typename T>
struct UnorderedKeyWrapper {
  /// The wrapped value
  const T value;
  /// The hash of value
  const size_t hash;

  /// Constructor
  /// @param v the value to wrap
  explicit UnorderedKeyWrapper(const T& v) : value(v), hash(Hash(v)) {}

  /// Move constructor
  /// @param v the value to wrap
  explicit UnorderedKeyWrapper(T&& v)
      : value(std::move(v)), hash(Hash(value)) {}

  /// @returns true if this wrapper comes before other
  /// @param other the RHS of the operator
  bool operator<(const UnorderedKeyWrapper& other) const {
    return hash < other.hash;
  }

  /// @returns true if this wrapped value is equal to the other wrapped value
  /// @param other the RHS of the operator
  bool operator==(const UnorderedKeyWrapper& other) const {
    return value == other.value;
  }
};

}  // namespace tint::utils

namespace std {

/// Custom std::hash specialization for tint::utils::UnorderedKeyWrapper
template <typename T>
class hash<tint::utils::UnorderedKeyWrapper<T>> {
 public:
  /// @param w the UnorderedKeyWrapper
  /// @return the hash value
  inline std::size_t operator()(
      const tint::utils::UnorderedKeyWrapper<T>& w) const {
    return w.hash;
  }
};

}  // namespace std

#endif  // SRC_TINT_UTILS_HASH_H_
