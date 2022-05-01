// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

struct BinaryData {
    ast::BinaryOp op;
    std::string name;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    out << data.op;
    return out;
}

using BinaryArithSignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryArithSignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3);
    auto* rhs = Expr(4);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithSignedIntegerTest, Vector) {
    auto param = GetParam();

    // Skip ops that are illegal for this type
    if (param.op == ast::BinaryOp::kAnd || param.op == ast::BinaryOp::kOr ||
        param.op == ast::BinaryOp::kXor) {
        return;
    }

    auto* lhs = vec3<i32>(1, 1, 1);
    auto* rhs = vec3<i32>(1, 1, 1);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
TEST_P(BinaryArithSignedIntegerTest, Scalar_Loads) {
    auto param = GetParam();

    auto* var = Var("param", ty.i32());
    auto* expr = create<ast::BinaryExpression>(param.op, Expr("param"), Expr("param"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateBinaryExpression(expr), 7u) << b.error();
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
              R"(%1 = OpVariable %2 Function %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%5 = OpLoad %3 %1
%6 = OpLoad %3 %1
%7 = )" + param.name +
                  R"( %3 %5 %6
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithSignedIntegerTest,
                         // NOTE: No left and right shift as they require u32 for rhs operand
                         testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpIAdd"},
                                         BinaryData{ast::BinaryOp::kAnd, "OpBitwiseAnd"},
                                         BinaryData{ast::BinaryOp::kDivide, "OpSDiv"},
                                         BinaryData{ast::BinaryOp::kModulo, "OpSMod"},
                                         BinaryData{ast::BinaryOp::kMultiply, "OpIMul"},
                                         BinaryData{ast::BinaryOp::kOr, "OpBitwiseOr"},
                                         BinaryData{ast::BinaryOp::kSubtract, "OpISub"},
                                         BinaryData{ast::BinaryOp::kXor, "OpBitwiseXor"}));

using BinaryArithUnsignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryArithUnsignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3u);
    auto* rhs = Expr(4u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}
TEST_P(BinaryArithUnsignedIntegerTest, Vector) {
    auto param = GetParam();

    // Skip ops that are illegal for this type
    if (param.op == ast::BinaryOp::kAnd || param.op == ast::BinaryOp::kOr ||
        param.op == ast::BinaryOp::kXor) {
        return;
    }

    auto* lhs = vec3<u32>(1u, 1u, 1u);
    auto* rhs = vec3<u32>(1u, 1u, 1u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryArithUnsignedIntegerTest,
    testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpIAdd"},
                    BinaryData{ast::BinaryOp::kAnd, "OpBitwiseAnd"},
                    BinaryData{ast::BinaryOp::kDivide, "OpUDiv"},
                    BinaryData{ast::BinaryOp::kModulo, "OpUMod"},
                    BinaryData{ast::BinaryOp::kMultiply, "OpIMul"},
                    BinaryData{ast::BinaryOp::kOr, "OpBitwiseOr"},
                    BinaryData{ast::BinaryOp::kShiftLeft, "OpShiftLeftLogical"},
                    BinaryData{ast::BinaryOp::kShiftRight, "OpShiftRightLogical"},
                    BinaryData{ast::BinaryOp::kSubtract, "OpISub"},
                    BinaryData{ast::BinaryOp::kXor, "OpBitwiseXor"}));

using BinaryArithFloatTest = TestParamHelper<BinaryData>;
TEST_P(BinaryArithFloatTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3.2f);
    auto* rhs = Expr(4.5f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithFloatTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
    auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithFloatTest,
                         testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpFAdd"},
                                         BinaryData{ast::BinaryOp::kDivide, "OpFDiv"},
                                         BinaryData{ast::BinaryOp::kModulo, "OpFRem"},
                                         BinaryData{ast::BinaryOp::kMultiply, "OpFMul"},
                                         BinaryData{ast::BinaryOp::kSubtract, "OpFSub"}));

using BinaryOperatorBoolTest = TestParamHelper<BinaryData>;
TEST_P(BinaryOperatorBoolTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(true);
    auto* rhs = Expr(false);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantFalse %1
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryOperatorBoolTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<bool>(false, true, false);
    auto* rhs = vec3<bool>(true, false, true);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 7u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 3
%3 = OpConstantFalse %2
%4 = OpConstantTrue %2
%5 = OpConstantComposite %1 %3 %4 %3
%6 = OpConstantComposite %1 %4 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%7 = " + param.name + " %1 %5 %6\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryOperatorBoolTest,
                         testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpLogicalEqual"},
                                         BinaryData{ast::BinaryOp::kNotEqual, "OpLogicalNotEqual"},
                                         BinaryData{ast::BinaryOp::kAnd, "OpLogicalAnd"},
                                         BinaryData{ast::BinaryOp::kOr, "OpLogicalOr"}));

using BinaryCompareUnsignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryCompareUnsignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3u);
    auto* rhs = Expr(4u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareUnsignedIntegerTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<u32>(1u, 1u, 1u);
    auto* rhs = vec3<u32>(1u, 1u, 1u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareUnsignedIntegerTest,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
                    BinaryData{ast::BinaryOp::kGreaterThan, "OpUGreaterThan"},
                    BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpUGreaterThanEqual"},
                    BinaryData{ast::BinaryOp::kLessThan, "OpULessThan"},
                    BinaryData{ast::BinaryOp::kLessThanEqual, "OpULessThanEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpINotEqual"}));

using BinaryCompareSignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryCompareSignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3);
    auto* rhs = Expr(4);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareSignedIntegerTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<i32>(1, 1, 1);
    auto* rhs = vec3<i32>(1, 1, 1);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareSignedIntegerTest,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
                    BinaryData{ast::BinaryOp::kGreaterThan, "OpSGreaterThan"},
                    BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpSGreaterThanEqual"},
                    BinaryData{ast::BinaryOp::kLessThan, "OpSLessThan"},
                    BinaryData{ast::BinaryOp::kLessThanEqual, "OpSLessThanEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpINotEqual"}));

using BinaryCompareFloatTest = TestParamHelper<BinaryData>;
TEST_P(BinaryCompareFloatTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3.2f);
    auto* rhs = Expr(4.5f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareFloatTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
    auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareFloatTest,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
                    BinaryData{ast::BinaryOp::kGreaterThan, "OpFOrdGreaterThan"},
                    BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual"},
                    BinaryData{ast::BinaryOp::kLessThan, "OpFOrdLessThan"},
                    BinaryData{ast::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

TEST_F(BuilderTest, Binary_Multiply_VectorScalar) {
    auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
    auto* rhs = Expr(1.f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = OpVectorTimesScalar %1 %4 %3\n");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarVector) {
    auto* lhs = Expr(1.f);
    auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 1
%3 = OpTypeVector %1 3
%4 = OpConstantComposite %3 %2 %2 %2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              "%5 = OpVectorTimesScalar %3 %4 %2\n");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixScalar) {
    auto* var = Var("mat", ty.mat3x3<f32>());

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr(1.f));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 8u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%7 = OpConstant %5 1
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpLoad %3 %1
%8 = OpMatrixTimesScalar %3 %6 %7
)");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarMatrix) {
    auto* var = Var("mat", ty.mat3x3<f32>());

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr(1.f), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 8u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%6 = OpConstant %5 1
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%7 = OpLoad %3 %1
%8 = OpMatrixTimesScalar %3 %7 %6
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixVector) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), rhs);

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%7 = OpConstant %5 1
%8 = OpConstantComposite %4 %7 %7 %7
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpLoad %3 %1
%9 = OpMatrixTimesVector %4 %6 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_VectorMatrix) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* lhs = vec3<f32>(1.f, 1.f, 1.f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%6 = OpConstant %5 1
%7 = OpConstantComposite %4 %6 %6 %6
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%8 = OpLoad %3 %1
%9 = OpVectorTimesMatrix %4 %7 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixMatrix) {
    auto* var = Var("mat", ty.mat3x3<f32>());

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 8u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpLoad %3 %1
%7 = OpLoad %3 %1
%8 = OpMatrixTimesMatrix %3 %6 %7
)");
}

TEST_F(BuilderTest, Binary_LogicalAnd) {
    auto* lhs = create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(1), Expr(2));

    auto* rhs = create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(3), Expr(4));

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%6 = OpTypeBool
%9 = OpConstant %2 3
%10 = OpConstant %2 4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
%5 = OpIEqual %6 %3 %4
OpSelectionMerge %7 None
OpBranchConditional %5 %8 %7
%8 = OpLabel
%11 = OpIEqual %6 %9 %10
OpBranch %7
%7 = OpLabel
%12 = OpPhi %6 %5 %1 %11 %8
)");
}

TEST_F(BuilderTest, Binary_LogicalAnd_WithLoads) {
    auto* a_var = Global("a", ty.bool_(), ast::StorageClass::kPrivate, Expr(true));
    auto* b_var = Global("b", ty.bool_(), ast::StorageClass::kPrivate, Expr(false));

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    ASSERT_TRUE(b.GenerateGlobalVariable(a_var)) << b.error();
    ASSERT_TRUE(b.GenerateGlobalVariable(b_var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%5 = OpTypePointer Private %2
%4 = OpVariable %5 Private %3
%6 = OpConstantFalse %2
%7 = OpVariable %5 Private %6
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
%8 = OpLoad %2 %4
OpSelectionMerge %9 None
OpBranchConditional %8 %10 %9
%10 = OpLabel
%11 = OpLoad %2 %7
OpBranch %9
%9 = OpLabel
%12 = OpPhi %2 %8 %1 %11 %10
)");
}

TEST_F(BuilderTest, Binary_logicalOr_Nested_LogicalAnd) {
    // Test an expression like
    //    a || (b && c)
    // From: crbug.com/tint/355

    auto* logical_and_expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr(true), Expr(false));

    auto* expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr(true), logical_and_expr);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%8 = OpConstantFalse %2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
OpSelectionMerge %4 None
OpBranchConditional %3 %4 %5
%5 = OpLabel
OpSelectionMerge %6 None
OpBranchConditional %3 %7 %6
%7 = OpLabel
OpBranch %6
%6 = OpLabel
%9 = OpPhi %2 %3 %5 %8 %7
OpBranch %4
%4 = OpLabel
%10 = OpPhi %2 %3 %1 %9 %6
)");
}

TEST_F(BuilderTest, Binary_logicalAnd_Nested_LogicalOr) {
    // Test an expression like
    //    a && (b || c)
    // From: crbug.com/tint/355

    auto* logical_or_expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr(true), Expr(false));

    auto* expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr(true), logical_or_expr);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%8 = OpConstantFalse %2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
OpSelectionMerge %4 None
OpBranchConditional %3 %5 %4
%5 = OpLabel
OpSelectionMerge %6 None
OpBranchConditional %3 %6 %7
%7 = OpLabel
OpBranch %6
%6 = OpLabel
%9 = OpPhi %2 %3 %5 %8 %7
OpBranch %4
%4 = OpLabel
%10 = OpPhi %2 %3 %1 %9 %6
)");
}

TEST_F(BuilderTest, Binary_LogicalOr) {
    auto* lhs = create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(1), Expr(2));

    auto* rhs = create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(3), Expr(4));

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()),
              R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%6 = OpTypeBool
%9 = OpConstant %2 3
%10 = OpConstant %2 4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
%5 = OpIEqual %6 %3 %4
OpSelectionMerge %7 None
OpBranchConditional %5 %7 %8
%8 = OpLabel
%11 = OpIEqual %6 %9 %10
OpBranch %7
%7 = OpLabel
%12 = OpPhi %6 %5 %1 %11 %8
)");
}

TEST_F(BuilderTest, Binary_LogicalOr_WithLoads) {
    auto* a_var = Global("a", ty.bool_(), ast::StorageClass::kPrivate, Expr(true));
    auto* b_var = Global("b", ty.bool_(), ast::StorageClass::kPrivate, Expr(false));

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    b.GenerateLabel(b.next_id());

    ASSERT_TRUE(b.GenerateGlobalVariable(a_var)) << b.error();
    ASSERT_TRUE(b.GenerateGlobalVariable(b_var)) << b.error();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%5 = OpTypePointer Private %2
%4 = OpVariable %5 Private %3
%6 = OpConstantFalse %2
%7 = OpVariable %5 Private %6
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpLabel
%8 = OpLoad %2 %4
OpSelectionMerge %9 None
OpBranchConditional %8 %9 %10
%10 = OpLabel
%11 = OpLoad %2 %7
OpBranch %9
%9 = OpLabel
%12 = OpPhi %2 %8 %1 %11 %10
)");
}

namespace BinaryArithVectorScalar {

enum class Type { f32, i32, u32 };
static const ast::Expression* MakeVectorExpr(ProgramBuilder* builder, Type type) {
    switch (type) {
        case Type::f32:
            return builder->vec3<ProgramBuilder::f32>(1.f, 1.f, 1.f);
        case Type::i32:
            return builder->vec3<ProgramBuilder::i32>(1, 1, 1);
        case Type::u32:
            return builder->vec3<ProgramBuilder::u32>(1u, 1u, 1u);
    }
    return nullptr;
}
static const ast::Expression* MakeScalarExpr(ProgramBuilder* builder, Type type) {
    switch (type) {
        case Type::f32:
            return builder->Expr(1.f);
        case Type::i32:
            return builder->Expr(1);
        case Type::u32:
            return builder->Expr(1u);
    }
    return nullptr;
}
static std::string OpTypeDecl(Type type) {
    switch (type) {
        case Type::f32:
            return "OpTypeFloat 32";
        case Type::i32:
            return "OpTypeInt 32 1";
        case Type::u32:
            return "OpTypeInt 32 0";
    }
    return {};
}

struct Param {
    Type type;
    ast::BinaryOp op;
    std::string name;
};

using BinaryArithVectorScalarTest = TestParamHelper<Param>;
TEST_P(BinaryArithVectorScalarTest, VectorScalar) {
    auto& param = GetParam();

    const ast::Expression* lhs = MakeVectorExpr(this, param.type);
    const ast::Expression* rhs = MakeScalarExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = )" + op_type_decl + R"(
%5 = OpTypeVector %6 3
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7 %7
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
%13 = OpCompositeConstruct %5 %7 %7 %7
%9 = )" + param.name + R"( %5 %8 %13
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
TEST_P(BinaryArithVectorScalarTest, ScalarVector) {
    auto& param = GetParam();

    const ast::Expression* lhs = MakeScalarExpr(this, param.type);
    const ast::Expression* rhs = MakeVectorExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = )" + op_type_decl + R"(
%6 = OpConstant %5 1
%7 = OpTypeVector %5 3
%8 = OpConstantComposite %7 %6 %6 %6
%11 = OpTypePointer Function %7
%12 = OpConstantNull %7
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
%13 = OpCompositeConstruct %7 %6 %6 %6
%9 = )" + param.name + R"( %7 %13 %8
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithVectorScalarTest,
                         testing::Values(Param{Type::f32, ast::BinaryOp::kAdd, "OpFAdd"},
                                         Param{Type::f32, ast::BinaryOp::kDivide, "OpFDiv"},
                                         // NOTE: Modulo not allowed on mixed float scalar-vector
                                         // Param{Type::f32, ast::BinaryOp::kModulo, "OpFMod"},
                                         // NOTE: We test f32 multiplies separately as we emit
                                         // OpVectorTimesScalar for this case
                                         // Param{Type::i32, ast::BinaryOp::kMultiply, "OpIMul"},
                                         Param{Type::f32, ast::BinaryOp::kSubtract, "OpFSub"},

                                         Param{Type::i32, ast::BinaryOp::kAdd, "OpIAdd"},
                                         Param{Type::i32, ast::BinaryOp::kDivide, "OpSDiv"},
                                         Param{Type::i32, ast::BinaryOp::kModulo, "OpSMod"},
                                         Param{Type::i32, ast::BinaryOp::kMultiply, "OpIMul"},
                                         Param{Type::i32, ast::BinaryOp::kSubtract, "OpISub"},

                                         Param{Type::u32, ast::BinaryOp::kAdd, "OpIAdd"},
                                         Param{Type::u32, ast::BinaryOp::kDivide, "OpUDiv"},
                                         Param{Type::u32, ast::BinaryOp::kModulo, "OpUMod"},
                                         Param{Type::u32, ast::BinaryOp::kMultiply, "OpIMul"},
                                         Param{Type::u32, ast::BinaryOp::kSubtract, "OpISub"}));

using BinaryArithVectorScalarMultiplyTest = TestParamHelper<Param>;
TEST_P(BinaryArithVectorScalarMultiplyTest, VectorScalar) {
    auto& param = GetParam();

    const ast::Expression* lhs = MakeVectorExpr(this, param.type);
    const ast::Expression* rhs = MakeScalarExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = )" + op_type_decl + R"(
%5 = OpTypeVector %6 3
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7 %7
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVectorTimesScalar %5 %8 %7
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
TEST_P(BinaryArithVectorScalarMultiplyTest, ScalarVector) {
    auto& param = GetParam();

    const ast::Expression* lhs = MakeScalarExpr(this, param.type);
    const ast::Expression* rhs = MakeVectorExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = )" + op_type_decl + R"(
%6 = OpConstant %5 1
%7 = OpTypeVector %5 3
%8 = OpConstantComposite %7 %6 %6 %6
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVectorTimesScalar %7 %8 %6
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithVectorScalarMultiplyTest,
                         testing::Values(Param{Type::f32, ast::BinaryOp::kMultiply, "OpFMul"}));

}  // namespace BinaryArithVectorScalar

namespace BinaryArithMatrixMatrix {

struct Param {
    ast::BinaryOp op;
    std::string name;
};

using BinaryArithMatrixMatrix = TestParamHelper<Param>;
TEST_P(BinaryArithMatrixMatrix, AddOrSubtract) {
    auto& param = GetParam();

    const ast::Expression* lhs = mat3x4<f32>();
    const ast::Expression* rhs = mat3x4<f32>();

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 4
%5 = OpTypeMatrix %6 3
%8 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpCompositeExtract %6 %8 0
%11 = OpCompositeExtract %6 %8 0
%12 = )" + param.name + R"( %6 %10 %11
%13 = OpCompositeExtract %6 %8 1
%14 = OpCompositeExtract %6 %8 1
%15 = )" + param.name + R"( %6 %13 %14
%16 = OpCompositeExtract %6 %8 2
%17 = OpCompositeExtract %6 %8 2
%18 = )" + param.name + R"( %6 %16 %17
%19 = OpCompositeConstruct %5 %12 %15 %18
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(  //
    BuilderTest,
    BinaryArithMatrixMatrix,
    testing::Values(Param{ast::BinaryOp::kAdd, "OpFAdd"},
                    Param{ast::BinaryOp::kSubtract, "OpFSub"}));

using BinaryArithMatrixMatrixMultiply = TestParamHelper<Param>;
TEST_P(BinaryArithMatrixMatrixMultiply, Multiply) {
    auto& param = GetParam();

    const ast::Expression* lhs = mat3x4<f32>();
    const ast::Expression* rhs = mat4x3<f32>();

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 4
%5 = OpTypeMatrix %6 3
%8 = OpConstantNull %5
%10 = OpTypeVector %7 3
%9 = OpTypeMatrix %10 4
%11 = OpConstantNull %9
%13 = OpTypeMatrix %6 4
%3 = OpFunction %2 None %1
%4 = OpLabel
%12 = OpMatrixTimesMatrix %13 %8 %11
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(  //
    BuilderTest,
    BinaryArithMatrixMatrixMultiply,
    testing::Values(Param{ast::BinaryOp::kMultiply, "OpFMul"}));

}  // namespace BinaryArithMatrixMatrix

}  // namespace
}  // namespace tint::writer::spirv
