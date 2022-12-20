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

#include <cstddef>
#include <cstring>
#include <type_traits>

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
    // gcc warns in cases where either TO or FROM are classes, even if they are trivially
    // copyable, with for example:
    //
    // error: ‘void* memcpy(void*, const void*, size_t)’ copying an object of
    // non-trivial type ‘struct tint::Number<unsigned int>’ from an array of ‘float’
    // [-Werror=class-memaccess]
    //
    // We avoid this by asserting that both types are indeed trivially copyable, and casting both
    // args to std::byte*.
    static_assert(std::is_trivially_copyable_v<std::decay_t<FROM>>);
    static_assert(std::is_trivially_copyable_v<std::decay_t<TO>>);
    TO to;
    memcpy(reinterpret_cast<std::byte*>(&to), reinterpret_cast<const std::byte*>(&from),
           sizeof(TO));
    return to;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_BITCAST_H_
