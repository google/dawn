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

#ifndef SRC_TINT_UTILS_BITCAST_H_
#define SRC_TINT_UTILS_BITCAST_H_

#include <cstring>

namespace tint::utils {

/// Bitcast performs a cast of `from` to the `TO` type using a memcpy.
/// This unsafe cast avoids triggering Clang's Control Flow Integrity checks.
/// See: crbug.com/dawn/1406
/// See: https://clang.llvm.org/docs/ControlFlowIntegrity.html#bad-cast-checking
/// @param from the value to cast
/// @tparam TO the value to cast to
/// @returns the cast value
template <typename TO, typename FROM>
inline TO Bitcast(FROM&& from) {
    static_assert(sizeof(FROM) == sizeof(TO));
    TO to;
    memcpy(&to, &from, sizeof(TO));
    return to;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_BITCAST_H_
