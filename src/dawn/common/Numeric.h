// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_NUMERIC_H_
#define SRC_DAWN_COMMON_NUMERIC_H_

#include <limits>

namespace detail {

    template <typename T>
    inline constexpr uint32_t u32_sizeof() {
        static_assert(sizeof(T) <= std::numeric_limits<uint32_t>::max());
        return uint32_t(sizeof(T));
    }

    template <typename T>
    inline constexpr uint32_t u32_alignof() {
        static_assert(alignof(T) <= std::numeric_limits<uint32_t>::max());
        return uint32_t(alignof(T));
    }

}  // namespace detail

template <typename T>
inline constexpr uint32_t u32_sizeof = detail::u32_sizeof<T>();

template <typename T>
inline constexpr uint32_t u32_alignof = detail::u32_alignof<T>();

#endif  // SRC_DAWN_COMMON_NUMERIC_H_
