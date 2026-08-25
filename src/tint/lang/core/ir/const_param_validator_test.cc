// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/const_param_validator.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

// These unit tests are used for internal development. CTS validation does a more complete job of
// testing all expectations for const and override parameters.

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ConstParamValidatorTest : public IRTestHelper {
  protected:
    void SetUp() override { mod.properties.Add(Property::kAllow16BitFloats); }
};

TEST_F(IR_ConstParamValidatorTest, CorrectDomainShiftLeft) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), e1, 17_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = shl %b, 17u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainShiftLeft) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), e1, 32_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = shl %b, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        "5:7 error: shift left value must be less than the bit width of the lhs, which is 32");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainShiftRight_Vec) {
    auto* func = b.Function("foo", ty.vec4i());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4i(), 4_i));
        auto* e2 = b.Splat(ty.vec4u(), 33_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.vec4i(), e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<i32> {
  $B1: {
    %b:vec4<i32> = let vec4<i32>(4i)
    %3:vec4<i32> = shl %b, vec4<u32>(33u)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        "5:7 error: shift left value must be less than the bit width of the lhs, which is 32");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainDiv) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kDivide, ty.u32(), e1, 17_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = div %b, 17u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainDiv) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kDivide, ty.u32(), e1, 0_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = div %b, 0u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, "5:7 error: integer division by zero is invalid");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainModulo_Vec) {
    auto* func = b.Function("foo", ty.vec4i());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4i(), 4_i));
        auto* e2 = b.Splat(ty.vec4i(), 0_i);
        auto* call_func = b.Binary(core::BinaryOp::kModulo, ty.vec4i(), e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result());
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<i32> {
  $B1: {
    %b:vec4<i32> = let vec4<i32>(4i)
    %3:vec4<i32> = mod %b, vec4<i32>(0i)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, "5:7 error: integer division by zero is invalid");
}

}  // namespace
}  // namespace tint::core::ir
