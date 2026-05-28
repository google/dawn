// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_NUMERIC_H_
#define SRC_DAWN_COMMON_NUMERIC_H_

#include <concepts>
#include <cstdint>
#include <limits>

#include "src/dawn/common/Assert.h"
#include "src/utils/underlying_type.h"

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

// Checked conversion between differently sized integer-likes (i.e. all the types that can be used
// by ityp: integers, TypedIntegers, and enum classes). This is only defined for unsigned types
// because that is all that is needed at the time of writing, however eventually we will want to use
// this more widely, and we'll need to upgrade it (and the tests) to allow signed types.
template <typename Dst, typename Src>
    requires HasUnsignedUnderlyingType<Src> && HasUnsignedUnderlyingType<Dst>
constexpr inline Dst checked_cast(const Src& value) {
    using ISrc = UnderlyingType<Src>;
    using IDst = UnderlyingType<Dst>;
    // The compiler seems to be able to optimize away this CHECK, for Src/Dst pairs that can never
    // fail (verified in Compiler Explorer with plain integers and enum classes).
    ISrc valueISrc = static_cast<ISrc>(value);
    DAWN_CHECK(valueISrc <= std::numeric_limits<IDst>::max());
    return Dst{static_cast<IDst>(valueISrc)};
}

// Returns if two inclusive integral ranges [x0, x1] and [y0, y1] have overlap.
template <typename T>
    requires std::integral<T>
bool RangesOverlap(T x0, T x1, T y0, T y1) {
    DAWN_ASSERT(x0 <= x1 && y0 <= y1);
    // Two ranges DON'T have overlap if and only if:
    // 1. [x0, x1] [y0, y1], or
    // 2. [y0, y1] [x0, x1]
    // which is (x1 < y0 || y1 < x0)
    // The inverse of which ends in the following statement.
    return x0 <= y1 && y0 <= x1;
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_NUMERIC_H_
