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

#ifndef SRC_TINT_UTILS_REVERSE_H_
#define SRC_TINT_UTILS_REVERSE_H_

#include <iterator>

namespace tint::utils {

namespace detail {
/// Used by utils::Reverse to hold the underlying iterable.
/// begin(ReverseIterable<T>) and end(ReverseIterable<T>) are automatically
/// called for range-for loops, via argument-dependent lookup.
/// See https://en.cppreference.com/w/cpp/language/range-for
template <typename T>
struct ReverseIterable {
    /// The wrapped iterable object.
    T& iterable;
};

template <typename T>
auto begin(ReverseIterable<T> r_it) {
    return std::rbegin(r_it.iterable);
}

template <typename T>
auto end(ReverseIterable<T> r_it) {
    return std::rend(r_it.iterable);
}
}  // namespace detail

/// Reverse returns an iterable wrapper that when used in range-for loops,
/// performs a reverse iteration over the object `iterable`.
/// Example:
/// ```
/// /* Equivalent to:
///  * for (auto it = vec.rbegin(); i != vec.rend(); ++it) {
///  *   auto v = *it;
///  */
/// for (auto v : utils::Reverse(vec)) {
/// }
/// ```
template <typename T>
detail::ReverseIterable<T> Reverse(T&& iterable) {
    return {iterable};
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_REVERSE_H_
