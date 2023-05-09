// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_PREDICATES_H_
#define SRC_TINT_UTILS_PREDICATES_H_

namespace tint::utils {

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// equal to
/// @p value
template <typename T>
auto Eq(const T& value) {
    return [value](const T& v) { return v == value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is not
/// equal to @p value
template <typename T>
auto Ne(const T& value) {
    return [value](const T& v) { return v != value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// greater than @p value
template <typename T>
auto Gt(const T& value) {
    return [value](const T& v) { return v > value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// less than
/// @p value
template <typename T>
auto Lt(const T& value) {
    return [value](const T& v) { return v < value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// greater or equal to @p value
template <typename T>
auto Ge(const T& value) {
    return [value](const T& v) { return v >= value; };
}

/// @param value the value to compare against
/// @return a function with the signature `bool(const T&)` which returns true if the argument is
/// less than or equal to @p value
template <typename T>
auto Le(const T& value) {
    return [value](const T& v) { return v <= value; };
}

/// @param ptr the pointer
/// @return true if the pointer argument is null.
static inline bool IsNull(const void* ptr) {
    return ptr == nullptr;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_PREDICATES_H_
