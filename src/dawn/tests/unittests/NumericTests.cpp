// Copyright 2023 The Dawn & Tint Authors
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

#include <limits>

#include "gtest/gtest.h"
#include "src/dawn/common/Numeric.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

// Tests for RangesOverlap
TEST(NumericTest, RangesOverlap) {
    // Range contains only one number
    ASSERT_EQ(true, RangesOverlap(0, 0, 0, 0));
    ASSERT_EQ(false, RangesOverlap(0, 0, 1, 1));

    // [  ]
    //      [  ]
    ASSERT_EQ(false, RangesOverlap(0, 8, 9, 16));

    //      [  ]
    // [  ]
    ASSERT_EQ(false, RangesOverlap(9, 16, 0, 8));

    //      [  ]
    // [             ]
    ASSERT_EQ(true, RangesOverlap(2, 3, 0, 8));

    // [             ]
    //      [  ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 2, 3));

    // [   ]
    //   [   ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 4, 12));

    //   [   ]
    // [   ]
    ASSERT_EQ(true, RangesOverlap(4, 12, 0, 8));

    // [   ]
    //     [   ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 8, 12));

    //     [   ]
    // [   ]
    ASSERT_EQ(true, RangesOverlap(8, 12, 0, 8));

    // Negative numbers
    ASSERT_EQ(true, RangesOverlap(-9, 12, 4, 16));
    ASSERT_EQ(false, RangesOverlap(-9, -3, -2, 0));
}

// Death tests
// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming

// Test for checked cast between various types.
template <typename T32, typename T64>
void CheckedCastTest() {
    using I32 = UnderlyingType<T32>;
    using I64 = UnderlyingType<T64>;
    static_assert(std::is_same_v<I32, uint32_t>);
    static_assert(std::is_same_v<I64, uint64_t>);

    // No-ops
    checked_cast<T32>(T32{std::numeric_limits<I32>::max()});
    checked_cast<T64>(T64{std::numeric_limits<I64>::max()});

    // Widening
    checked_cast<T64>(T32{std::numeric_limits<I32>::max()});

    // Narrowing
    EXPECT_DEATH(checked_cast<T32>(T64{uint64_t{std::numeric_limits<I32>::max()} + 1}), "");
    EXPECT_DEATH(checked_cast<T32>(T64{uint64_t{std::numeric_limits<I32>::max()} * 2}), "");
    EXPECT_DEATH(checked_cast<T32>(T64{std::numeric_limits<uint64_t>::max()}), "");
}
TEST(NumericDeathTest, CheckedCast) {
    CheckedCastTest<uint32_t, uint64_t>();

    using TypedU32 = TypedInteger<struct TypedU32T, uint32_t>;
    using TypedU64 = TypedInteger<struct TypedU64T, uint64_t>;
    CheckedCastTest<uint32_t, TypedU64>();
    CheckedCastTest<TypedU32, uint64_t>();
    CheckedCastTest<TypedU32, TypedU64>();

    enum class EnumU32 : uint32_t {};
    enum class EnumU64 : uint64_t {};
    CheckedCastTest<uint32_t, EnumU64>();
    CheckedCastTest<EnumU32, uint64_t>();
    CheckedCastTest<EnumU32, EnumU64>();
}

}  // anonymous namespace
}  // namespace dawn
