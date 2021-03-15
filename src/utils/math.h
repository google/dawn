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

#ifndef SRC_UTILS_MATH_H_
#define SRC_UTILS_MATH_H_

#include <sstream>
#include <string>

namespace tint {
namespace utils {

/// @param alignment the next multiple to round `value` to
/// @param value the value to round to the next multiple of `alignment`
/// @return `value` rounded to the next multiple of `alignment`
/// @note `alignment` must be positive. An alignment of zero will cause a DBZ.
template <typename T>
inline T RoundUp(T alignment, T value) {
  return ((value + alignment - 1) / alignment) * alignment;
}

/// @param value the value to check whether it is a power-of-two
/// @returns true if `value` is a power-of-two
/// @note `value` must be positive if `T` is signed
template <typename T>
inline bool IsPowerOfTwo(T value) {
  return (value & (value - 1)) == 0;
}

}  // namespace utils
}  // namespace tint

#endif  //  SRC_UTILS_MATH_H_
