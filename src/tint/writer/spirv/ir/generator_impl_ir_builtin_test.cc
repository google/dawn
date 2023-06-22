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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

#include "gmock/gmock.h"
#include "src/tint/builtin/function.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

/// A parameterized builtin function test case.
struct BuiltinTestCase {
    /// The element type to test.
    TestElementType type;
    /// The builtin function.
    enum builtin::Function function;
    /// The expected SPIR-V instruction string.
    std::string spirv_inst;
};

// Tests for builtins with the signature: T = func(T)
using Builtin_1arg = SpvGeneratorImplTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_1arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
TEST_P(Builtin_1arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         Builtin_1arg,
                         testing::Values(BuiltinTestCase{kI32, builtin::Function::kAbs, "SAbs"},
                                         BuiltinTestCase{kF32, builtin::Function::kAbs, "FAbs"}));

// Test that abs of an unsigned value just folds away.
TEST_F(SpvGeneratorImplTest, Builtin_Abs_u32) {
    auto* func = b.Function("foo", MakeScalarType(kU32));
    b.With(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kU32), builtin::Function::kAbs, MakeScalarValue(kU32));
        b.Return(func, result);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%5 = OpConstant %2 1
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturnValue %5
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Builtin_Abs_vec2u) {
    auto* func = b.Function("foo", MakeVectorType(kU32));
    b.With(func->Block(), [&] {
        auto* result = b.Call(MakeVectorType(kU32), builtin::Function::kAbs, MakeVectorValue(kU32));
        b.Return(func, result);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%3 = OpTypeInt 32 0
%2 = OpTypeVector %3 2
%4 = OpTypeFunction %2
%7 = OpConstant %3 42
%8 = OpConstant %3 10
%6 = OpConstantComposite %2 %7 %8
%1 = OpFunction %2 None %4
%5 = OpLabel
OpReturnValue %6
OpFunctionEnd
)");
}

// Tests for builtins with the signature: T = func(T, T)
using Builtin_2arg = SpvGeneratorImplTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_2arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type),
               MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
TEST_P(Builtin_2arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         Builtin_2arg,
                         testing::Values(BuiltinTestCase{kF32, builtin::Function::kMax, "FMax"},
                                         BuiltinTestCase{kI32, builtin::Function::kMax, "SMax"},
                                         BuiltinTestCase{kU32, builtin::Function::kMax, "UMax"},
                                         BuiltinTestCase{kF32, builtin::Function::kMin, "FMin"},
                                         BuiltinTestCase{kI32, builtin::Function::kMin, "SMin"},
                                         BuiltinTestCase{kU32, builtin::Function::kMin, "UMin"}));

}  // namespace
}  // namespace tint::writer::spirv
