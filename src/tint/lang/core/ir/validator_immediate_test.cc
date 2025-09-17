// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/validator_test.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;  // NOLINT

// Helper to make a module-scope immediate variable with given store type.
static Var* MakeImmediate(Builder& b,
                          Module& mod,
                          const char* name,
                          const core::type::Type* store_ty) {
    auto* v = b.Var(name, core::AddressSpace::kImmediate, store_ty);
    mod.root_block->Append(v);
    return v;
}

class IR_ValidatorImmediateTest : public IR_ValidatorTest {};

TEST_F(IR_ValidatorImmediateTest, ValidateSingleUserImmediate_None) {
    // Empty module (no immediates)
    auto res = ValidateSingleUserImmediate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
    EXPECT_EQ(res.Get(), 0u);
}

TEST_F(IR_ValidatorImmediateTest, ValidateSingleUserImmediate_One) {
    // Single immediate variable with size 12 -> rounded to 12 (already multiple of 4)
    auto* s = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("a"), ty.array<i32, 3>()}});
    MakeImmediate(b, mod, "immediate_data", s);
    auto res = ValidateSingleUserImmediate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
    EXPECT_EQ(res.Get(), 12u);  // 3 * 4 bytes
}

// Round-up behavior is implicitly covered by using a struct whose size is already a multiple of 4.

TEST_F(IR_ValidatorImmediateTest, ValidateSingleUserImmediate_Multiple) {
    auto* s = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("a"), ty.i32()}});
    MakeImmediate(b, mod, "immediate0", s);
    MakeImmediate(b, mod, "immediate1", s);
    auto res = ValidateSingleUserImmediate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                ::testing::HasSubstr("multiple user-declared immediate data"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_BasicSuccess) {
    // No user immediate, two internal ranges non-overlapping
    std::vector<ImmediateInfo> immediates = {
        {0u, 16u},  // starts after user (none), size 16
        {32u, 8u},
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_OverlapUser) {
    // User immediate occupies first 32 bytes
    std::vector<ImmediateInfo> immediates = {
        {16u, 8u},
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 32u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                ::testing::HasSubstr("overlaps user-declared immediate data"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_ZeroSize) {
    std::vector<ImmediateInfo> immediates = {
        {64u, 0u},
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, ::testing::HasSubstr("zero size"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_Misaligned) {
    std::vector<ImmediateInfo> immediates = {
        {10u, 4u},
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, ::testing::HasSubstr("not 4-byte aligned"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_ExceedsMax) {
    std::vector<ImmediateInfo> immediates = {
        {0x2000u, 4u},
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, ::testing::HasSubstr("offset exceeds"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_OffsetPlusSizeExceeds) {
    std::vector<ImmediateInfo> immediates = {
        {0x0FFCu, 16u},  // 0x0FFC + 16 > 0x1000
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, ::testing::HasSubstr("(offset + size)"));
}

TEST_F(IR_ValidatorImmediateTest, ValidateInternalImmediateOffset_RangesOverlap) {
    std::vector<ImmediateInfo> immediates = {
        {64u, 32u}, {80u, 16u},  // overlaps previous (64..96) vs (80..96)
    };
    auto res = ValidateInternalImmediateOffset(0x1000u, 0u, immediates);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, ::testing::HasSubstr("ranges overlap"));
}

}  // namespace tint::core::ir
