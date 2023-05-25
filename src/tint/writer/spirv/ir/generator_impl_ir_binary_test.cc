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
#include "src/tint/ir/binary.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

/// The element type of a test.
enum Type {
    kBool,
    kI32,
    kU32,
    kF32,
    kF16,
};

/// A parameterized test case.
struct BinaryTestCase {
    /// The element type to test.
    Type type;
    /// The binary operation.
    enum ir::Binary::Kind kind;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
};

/// A helper class for parameterized binary instruction tests.
class BinaryInstructionTest : public SpvGeneratorImplTestWithParam<BinaryTestCase> {
  protected:
    /// Helper to make a scalar type corresponding to the element type `ty`.
    /// @param ty the element type
    /// @returns the scalar type
    const type::Type* MakeScalarType(Type ty) {
        switch (ty) {
            case kBool:
                return mod.Types().bool_();
            case kI32:
                return mod.Types().i32();
            case kU32:
                return mod.Types().u32();
            case kF32:
                return mod.Types().f32();
            case kF16:
                return mod.Types().f16();
        }
        return nullptr;
    }

    /// Helper to make a vector type corresponding to the element type `ty`.
    /// @param ty the element type
    /// @returns the vector type
    const type::Type* MakeVectorType(Type ty) { return mod.Types().vec2(MakeScalarType(ty)); }

    /// Helper to make a scalar value with the scalar type `ty`.
    /// @param ty the element type
    /// @returns the scalar value
    ir::Value* MakeScalarValue(Type ty) {
        switch (ty) {
            case kBool:
                return b.Constant(true);
            case kI32:
                return b.Constant(1_i);
            case kU32:
                return b.Constant(1_u);
            case kF32:
                return b.Constant(1_f);
            case kF16:
                return b.Constant(1_h);
        }
        return nullptr;
    }

    /// Helper to make a vector value with an element type of `ty`.
    /// @param ty the element type
    /// @returns the vector value
    ir::Value* MakeVectorValue(Type ty) {
        switch (ty) {
            case kBool:
                return b.Constant(b.ir.constant_values.Composite(
                    MakeVectorType(ty), utils::Vector{b.ir.constant_values.Get(true),
                                                      b.ir.constant_values.Get(false)}));
            case kI32:
                return b.Constant(b.ir.constant_values.Composite(
                    MakeVectorType(ty), utils::Vector{b.ir.constant_values.Get(42_i),
                                                      b.ir.constant_values.Get(-10_i)}));
            case kU32:
                return b.Constant(b.ir.constant_values.Composite(
                    MakeVectorType(ty),
                    utils::Vector{b.ir.constant_values.Get(42_u), b.ir.constant_values.Get(10_u)}));
            case kF32:
                return b.Constant(b.ir.constant_values.Composite(
                    MakeVectorType(ty), utils::Vector{b.ir.constant_values.Get(42_f),
                                                      b.ir.constant_values.Get(-0.5_f)}));
            case kF16:
                return b.Constant(b.ir.constant_values.Composite(
                    MakeVectorType(ty), utils::Vector{b.ir.constant_values.Get(42_h),
                                                      b.ir.constant_values.Get(-0.5_h)}));
        }
        return nullptr;
    }
};

TEST_F(SpvGeneratorImplTest, Binary_Add_I32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(utils::Vector{
        b.Add(mod.Types().i32(), b.Constant(1_i), b.Constant(2_i)), b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_U32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(utils::Vector{
        b.Add(mod.Types().u32(), b.Constant(1_u), b.Constant(2_u)), b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_F32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(utils::Vector{
        b.Add(mod.Types().f32(), b.Constant(1_f), b.Constant(2_f)), b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_I32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.Types().i32(), b.Constant(1_i), b.Constant(2_i)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_U32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.Types().u32(), b.Constant(1_u), b.Constant(2_u)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_F32) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.Types().f32(), b.Constant(1_f), b.Constant(2_f)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFSub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_Vec2i) {
    auto const_i32 = [&](int val) { return b.ir.constant_values.Get(i32(val)); };
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    auto* lhs = b.ir.constant_values.Composite(mod.Types().vec2(mod.Types().i32()),
                                               utils::Vector{const_i32(42), const_i32(-1)});
    auto* rhs = b.ir.constant_values.Composite(mod.Types().vec2(mod.Types().i32()),
                                               utils::Vector{const_i32(0), const_i32(-43)});
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.Types().Get<type::Vector>(mod.Types().i32(), 2u),
                                 b.Constant(lhs), b.Constant(rhs)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 42
%10 = OpConstant %7 -1
%8 = OpConstantComposite %6 %9 %10
%12 = OpConstant %7 0
%13 = OpConstant %7 -43
%11 = OpConstantComposite %6 %12 %13
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %8 %11
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_Vec4f) {
    auto const_f32 = [&](float val) { return b.ir.constant_values.Get(f32(val)); };
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    auto* lhs = b.ir.constant_values.Composite(
        mod.Types().vec4(mod.Types().f32()),
        utils::Vector{const_f32(42), const_f32(-1), const_f32(0), const_f32(1.25)});
    auto* rhs = b.ir.constant_values.Composite(
        mod.Types().vec4(mod.Types().f32()),
        utils::Vector{const_f32(0), const_f32(1.25), const_f32(-42), const_f32(1)});
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.Types().Get<type::Vector>(mod.Types().f32(), 4u),
                                 b.Constant(lhs), b.Constant(rhs)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 4
%9 = OpConstant %7 42
%10 = OpConstant %7 -1
%11 = OpConstant %7 0
%12 = OpConstant %7 1.25
%8 = OpConstantComposite %6 %9 %10 %11 %12
%14 = OpConstant %7 -42
%15 = OpConstant %7 1
%13 = OpConstantComposite %6 %11 %12 %14 %15
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFSub %6 %8 %13
OpReturn
OpFunctionEnd
)");
}

using Comparison = BinaryInstructionTest;
TEST_P(Comparison, Scalar) {
    auto params = GetParam();

    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{b.CreateBinary(params.kind, mod.Types().bool_(), MakeScalarValue(params.type),
                                     MakeScalarValue(params.type)),
                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
TEST_P(Comparison, Vector) {
    auto params = GetParam();

    auto* func = b.CreateFunction("foo", mod.Types().void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{b.CreateBinary(params.kind, mod.Types().vec2(mod.Types().bool_()),
                                     MakeVectorValue(params.type), MakeVectorValue(params.type)),

                      b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_THAT(DumpModule(generator_.Module()), ::testing::HasSubstr(params.spirv_inst));
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest_Binary_I32,
    Comparison,
    testing::Values(BinaryTestCase{kI32, ir::Binary::Kind::kEqual, "OpIEqual"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kNotEqual, "OpINotEqual"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kGreaterThan, "OpSGreaterThan"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kGreaterThanEqual,
                                   "OpSGreaterThanEqual"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kLessThan, "OpSLessThan"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kLessThanEqual, "OpSLessThanEqual"}));
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest_Binary_U32,
    Comparison,
    testing::Values(BinaryTestCase{kU32, ir::Binary::Kind::kEqual, "OpIEqual"},
                    BinaryTestCase{kU32, ir::Binary::Kind::kNotEqual, "OpINotEqual"},
                    BinaryTestCase{kU32, ir::Binary::Kind::kGreaterThan, "OpUGreaterThan"},
                    BinaryTestCase{kU32, ir::Binary::Kind::kGreaterThanEqual,
                                   "OpUGreaterThanEqual"},
                    BinaryTestCase{kU32, ir::Binary::Kind::kLessThan, "OpULessThan"},
                    BinaryTestCase{kU32, ir::Binary::Kind::kLessThanEqual, "OpULessThanEqual"}));
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest_Binary_F32,
    Comparison,
    testing::Values(BinaryTestCase{kF32, ir::Binary::Kind::kEqual, "OpFOrdEqual"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kNotEqual, "OpFOrdNotEqual"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kGreaterThan, "OpFOrdGreaterThan"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kGreaterThanEqual,
                                   "OpFOrdGreaterThanEqual"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kLessThan, "OpFOrdLessThan"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kLessThanEqual, "OpFOrdLessThanEqual"}));
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest_Binary_F16,
    Comparison,
    testing::Values(BinaryTestCase{kF16, ir::Binary::Kind::kEqual, "OpFOrdEqual"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kNotEqual, "OpFOrdNotEqual"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kGreaterThan, "OpFOrdGreaterThan"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kGreaterThanEqual,
                                   "OpFOrdGreaterThanEqual"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kLessThan, "OpFOrdLessThan"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kLessThanEqual, "OpFOrdLessThanEqual"}));
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest_Binary_Bool,
    Comparison,
    testing::Values(BinaryTestCase{kBool, ir::Binary::Kind::kEqual, "OpLogicalEqual"},
                    BinaryTestCase{kBool, ir::Binary::Kind::kNotEqual, "OpLogicalNotEqual"}));

TEST_F(SpvGeneratorImplTest, Binary_Chain) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());
    auto* a = b.Subtract(mod.Types().i32(), b.Constant(1_i), b.Constant(2_i));
    func->StartTarget()->SetInstructions(
        utils::Vector{a, b.Add(mod.Types().i32(), a, a), b.Branch(func->EndTarget())});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
%9 = OpIAdd %6 %5 %5
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
