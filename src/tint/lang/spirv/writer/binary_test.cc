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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "src/tint/lang/core/ir/binary.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

/// A parameterized test case.
struct BinaryTestCase {
    /// The element type to test.
    TestElementType type;
    /// The binary operation.
    enum ir::Binary::Kind kind;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};

using Arithmetic_Bitwise = SpirvWriterTestWithParam<BinaryTestCase>;
TEST_P(Arithmetic_Bitwise, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.type);
        auto* rhs = MakeScalarValue(params.type);
        auto* result = b.Binary(params.kind, MakeScalarType(params.type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name);
}
TEST_P(Arithmetic_Bitwise, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.type);
        auto* rhs = MakeVectorValue(params.type);
        auto* result = b.Binary(params.kind, MakeVectorType(params.type), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_I32,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kI32, ir::Binary::Kind::kAdd, "OpIAdd", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kSubtract, "OpISub", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kMultiply, "OpIMul", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kDivide, "OpSDiv", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kModulo, "OpSRem", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kAnd, "OpBitwiseAnd", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kOr, "OpBitwiseOr", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kXor, "OpBitwiseXor", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kShiftLeft, "OpShiftLeftLogical", "int"},
                    BinaryTestCase{kI32, ir::Binary::Kind::kShiftRight, "OpShiftRightArithmetic",
                                   "int"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_U32,
    Arithmetic_Bitwise,
    testing::Values(
        BinaryTestCase{kU32, ir::Binary::Kind::kAdd, "OpIAdd", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kSubtract, "OpISub", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kMultiply, "OpIMul", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kDivide, "OpUDiv", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kModulo, "OpUMod", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kAnd, "OpBitwiseAnd", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kOr, "OpBitwiseOr", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kXor, "OpBitwiseXor", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kShiftLeft, "OpShiftLeftLogical", "uint"},
        BinaryTestCase{kU32, ir::Binary::Kind::kShiftRight, "OpShiftRightLogical", "uint"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F32,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kF32, ir::Binary::Kind::kAdd, "OpFAdd", "float"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kSubtract, "OpFSub", "float"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kMultiply, "OpFMul", "float"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kDivide, "OpFDiv", "float"},
                    BinaryTestCase{kF32, ir::Binary::Kind::kModulo, "OpFRem", "float"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F16,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kF16, ir::Binary::Kind::kAdd, "OpFAdd", "half"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kSubtract, "OpFSub", "half"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kMultiply, "OpFMul", "half"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kDivide, "OpFDiv", "half"},
                    BinaryTestCase{kF16, ir::Binary::Kind::kModulo, "OpFRem", "half"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_Bool,
    Arithmetic_Bitwise,
    testing::Values(BinaryTestCase{kBool, ir::Binary::Kind::kAnd, "OpLogicalAnd", "bool"},
                    BinaryTestCase{kBool, ir::Binary::Kind::kOr, "OpLogicalOr", "bool"}));

TEST_F(SpirvWriterTest, Binary_ScalarTimesVector_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), scalar, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, vector});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), vector, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesScalar %v4float %vector %scalar");
}

TEST_F(SpirvWriterTest, Binary_ScalarTimesMatrix_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x4<f32>(), scalar, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesScalar_F32) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({scalar, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x4<f32>(), matrix, scalar);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesScalar %mat3v4float %matrix %scalar");
}

TEST_F(SpirvWriterTest, Binary_VectorTimesMatrix_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec3<f32>(), vector, matrix);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorTimesMatrix %v3float %vector %matrix");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesVector_F32) {
    auto* vector = b.FunctionParam("vector", ty.vec3<f32>());
    auto* matrix = b.FunctionParam("matrix", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vector, matrix});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), matrix, vector);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesVector %v4float %matrix %vector");
}

TEST_F(SpirvWriterTest, Binary_MatrixTimesMatrix_F32) {
    auto* mat1 = b.FunctionParam("mat1", ty.mat4x3<f32>());
    auto* mat2 = b.FunctionParam("mat2", ty.mat3x4<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({mat1, mat2});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.mat3x3<f32>(), mat1, mat2);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpMatrixTimesMatrix %mat3v3float %mat1 %mat2");
}

using Comparison = SpirvWriterTestWithParam<BinaryTestCase>;
TEST_P(Comparison, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeScalarValue(params.type);
        auto* rhs = MakeScalarValue(params.type);
        auto* result = b.Binary(params.kind, ty.bool_(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %bool");
}

TEST_P(Comparison, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* lhs = MakeVectorValue(params.type);
        auto* rhs = MakeVectorValue(params.type);
        auto* result = b.Binary(params.kind, ty.vec2<bool>(), lhs, rhs);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2bool");
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_I32,
    Comparison,
    testing::Values(
        BinaryTestCase{kI32, ir::Binary::Kind::kEqual, "OpIEqual", "bool"},
        BinaryTestCase{kI32, ir::Binary::Kind::kNotEqual, "OpINotEqual", "bool"},
        BinaryTestCase{kI32, ir::Binary::Kind::kGreaterThan, "OpSGreaterThan", "bool"},
        BinaryTestCase{kI32, ir::Binary::Kind::kGreaterThanEqual, "OpSGreaterThanEqual", "bool"},
        BinaryTestCase{kI32, ir::Binary::Kind::kLessThan, "OpSLessThan", "bool"},
        BinaryTestCase{kI32, ir::Binary::Kind::kLessThanEqual, "OpSLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_U32,
    Comparison,
    testing::Values(
        BinaryTestCase{kU32, ir::Binary::Kind::kEqual, "OpIEqual", "bool"},
        BinaryTestCase{kU32, ir::Binary::Kind::kNotEqual, "OpINotEqual", "bool"},
        BinaryTestCase{kU32, ir::Binary::Kind::kGreaterThan, "OpUGreaterThan", "bool"},
        BinaryTestCase{kU32, ir::Binary::Kind::kGreaterThanEqual, "OpUGreaterThanEqual", "bool"},
        BinaryTestCase{kU32, ir::Binary::Kind::kLessThan, "OpULessThan", "bool"},
        BinaryTestCase{kU32, ir::Binary::Kind::kLessThanEqual, "OpULessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F32,
    Comparison,
    testing::Values(
        BinaryTestCase{kF32, ir::Binary::Kind::kEqual, "OpFOrdEqual", "bool"},
        BinaryTestCase{kF32, ir::Binary::Kind::kNotEqual, "OpFOrdNotEqual", "bool"},
        BinaryTestCase{kF32, ir::Binary::Kind::kGreaterThan, "OpFOrdGreaterThan", "bool"},
        BinaryTestCase{kF32, ir::Binary::Kind::kGreaterThanEqual, "OpFOrdGreaterThanEqual", "bool"},
        BinaryTestCase{kF32, ir::Binary::Kind::kLessThan, "OpFOrdLessThan", "bool"},
        BinaryTestCase{kF32, ir::Binary::Kind::kLessThanEqual, "OpFOrdLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Binary_F16,
    Comparison,
    testing::Values(
        BinaryTestCase{kF16, ir::Binary::Kind::kEqual, "OpFOrdEqual", "bool"},
        BinaryTestCase{kF16, ir::Binary::Kind::kNotEqual, "OpFOrdNotEqual", "bool"},
        BinaryTestCase{kF16, ir::Binary::Kind::kGreaterThan, "OpFOrdGreaterThan", "bool"},
        BinaryTestCase{kF16, ir::Binary::Kind::kGreaterThanEqual, "OpFOrdGreaterThanEqual", "bool"},
        BinaryTestCase{kF16, ir::Binary::Kind::kLessThan, "OpFOrdLessThan", "bool"},
        BinaryTestCase{kF16, ir::Binary::Kind::kLessThanEqual, "OpFOrdLessThanEqual", "bool"}));
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest_Binary_Bool,
                         Comparison,
                         testing::Values(BinaryTestCase{kBool, ir::Binary::Kind::kEqual,
                                                        "OpLogicalEqual", "bool"},
                                         BinaryTestCase{kBool, ir::Binary::Kind::kNotEqual,
                                                        "OpLogicalNotEqual", "bool"}));

TEST_F(SpirvWriterTest, Binary_Chain) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* sub = b.Subtract(ty.i32(), 1_i, 2_i);
        auto* add = b.Add(ty.i32(), sub, sub);
        b.Return(func);
        mod.SetName(sub, "sub");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%sub = OpISub %int %int_1 %int_2");
    EXPECT_INST("%add = OpIAdd %int %sub %sub");
}

}  // namespace
}  // namespace tint::spirv::writer
