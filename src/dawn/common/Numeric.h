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

#include <cstdint>
#include <limits>
#include <type_traits>

#include "dawn/common/Assert.h"

namespace dawn {
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

// Only defined for unsigned integers because that is all that is
// needed at the time of writing.
template <typename Dst, typename Src, typename = std::enable_if_t<std::is_unsigned_v<Src>>>
inline Dst checked_cast(const Src& value) {
    ASSERT(value <= std::numeric_limits<Dst>::max());
    return static_cast<Dst>(value);
}

template <typename T>
bool IsDoubleValueRepresentable(double value) {
    if constexpr (std::is_same_v<T, float> || std::is_integral_v<T>) {
        // Following WebIDL 3.3.6.[EnforceRange] for integral
        // Following WebIDL 3.2.5.float for float
        // TODO(crbug.com/1396194): now follows what blink does but may need revisit.
        constexpr double kLowest = static_cast<double>(std::numeric_limits<T>::lowest());
        constexpr double kMax = static_cast<double>(std::numeric_limits<T>::max());
        return kLowest <= value && value <= kMax;
    } else {
        static_assert(std::is_same_v<T, float> || std::is_integral_v<T>, "Unsupported type");
    }
}

// Returns if two inclusive integral ranges [x0, x1] and [y0, y1] have overlap.
template <typename T>
bool RangesOverlap(T x0, T x1, T y0, T y1) {
    ASSERT(x0 <= x1 && y0 <= y1);
    if constexpr (std::is_integral_v<T>) {
        // Two ranges DON'T have overlap if and only if:
        // 1. [x0, x1] [y0, y1], or
        // 2. [y0, y1] [x0, x1]
        // which is (x1 < y0 || y1 < x0)
        // The inverse of which ends in the following statement.
        return x0 <= y1 && y0 <= x1;
    } else {
        static_assert(std::is_integral_v<T>, "Unsupported type");
    }
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_NUMERIC_H_
