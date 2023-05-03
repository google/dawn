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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

struct BinaryData {
    ast::BinaryOp op;
    std::string name;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    utils::StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using BinaryArithSignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryArithSignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3_i);
    auto* rhs = Expr(4_i);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithSignedIntegerTest, Vector) {
    auto param = GetParam();

    // Skip ops that are illegal for this type
    if (param.op == ast::BinaryOp::kAnd || param.op == ast::BinaryOp::kOr ||
        param.op == ast::BinaryOp::kXor) {
        return;
    }

    auto* lhs = vec3<i32>(1_i, 1_i, 1_i);
    auto* rhs = vec3<i32>(1_i, 1_i, 1_i);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
TEST_P(BinaryArithSignedIntegerTest, Scalar_Loads) {
    auto param = GetParam();

    auto* var = Var("param", ty.i32());
    auto* expr = create<ast::BinaryExpression>(param.op, Expr("param"), Expr("param"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateBinaryExpression(expr), 7u) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().variables()),
              R"(%1 = OpVariable %2 Function %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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
                                         BinaryData{ast::BinaryOp::kModulo, "OpSRem"},
                                         BinaryData{ast::BinaryOp::kMultiply, "OpIMul"},
                                         BinaryData{ast::BinaryOp::kOr, "OpBitwiseOr"},
                                         BinaryData{ast::BinaryOp::kSubtract, "OpISub"},
                                         BinaryData{ast::BinaryOp::kXor, "OpBitwiseXor"}));

using BinaryArithUnsignedIntegerTest = TestParamHelper<BinaryData>;
TEST_P(BinaryArithUnsignedIntegerTest, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3_u);
    auto* rhs = Expr(4_u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}
TEST_P(BinaryArithUnsignedIntegerTest, Vector) {
    auto param = GetParam();

    // Skip ops that are illegal for this type
    if (param.op == ast::BinaryOp::kAnd || param.op == ast::BinaryOp::kOr ||
        param.op == ast::BinaryOp::kXor) {
        return;
    }

    auto* lhs = vec3<u32>(1_u, 1_u, 1_u);
    auto* rhs = vec3<u32>(1_u, 1_u, 1_u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

using BinaryArithF32Test = TestParamHelper<BinaryData>;
TEST_P(BinaryArithF32Test, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3.2_f);
    auto* rhs = Expr(4.5_f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithF32Test, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithF32Test,
                         testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpFAdd"},
                                         BinaryData{ast::BinaryOp::kDivide, "OpFDiv"},
                                         BinaryData{ast::BinaryOp::kModulo, "OpFRem"},
                                         BinaryData{ast::BinaryOp::kMultiply, "OpFMul"},
                                         BinaryData{ast::BinaryOp::kSubtract, "OpFSub"}));

using BinaryArithF16Test = TestParamHelper<BinaryData>;
TEST_P(BinaryArithF16Test, Scalar) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();

    auto* lhs = Expr(3.2_h);
    auto* rhs = Expr(4.5_h);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.998p+1
%3 = OpConstant %1 0x1.2p+2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithF16Test, Vector) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();

    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithF16Test,
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

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryOperatorBoolTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<bool>(false, true, false);
    auto* rhs = vec3<bool>(true, false, true);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 7u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 3
%3 = OpConstantNull %2
%4 = OpConstantTrue %2
%5 = OpConstantComposite %1 %3 %4 %3
%6 = OpConstantComposite %1 %4 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

    auto* lhs = Expr(3_u);
    auto* rhs = Expr(4_u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareUnsignedIntegerTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<u32>(1_u, 1_u, 1_u);
    auto* rhs = vec3<u32>(1_u, 1_u, 1_u);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

    auto* lhs = Expr(3_i);
    auto* rhs = Expr(4_i);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareSignedIntegerTest, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<i32>(1_i, 1_i, 1_i);
    auto* rhs = vec3<i32>(1_i, 1_i, 1_i);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

using BinaryCompareF32Test = TestParamHelper<BinaryData>;
TEST_P(BinaryCompareF32Test, Scalar) {
    auto param = GetParam();

    auto* lhs = Expr(3.2_f);
    auto* rhs = Expr(4.5_f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareF32Test, Vector) {
    auto param = GetParam();

    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareF32Test,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
                    BinaryData{ast::BinaryOp::kGreaterThan, "OpFOrdGreaterThan"},
                    BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual"},
                    BinaryData{ast::BinaryOp::kLessThan, "OpFOrdLessThan"},
                    BinaryData{ast::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

using BinaryCompareF16Test = TestParamHelper<BinaryData>;
TEST_P(BinaryCompareF16Test, Scalar) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();

    auto* lhs = Expr(3.2_h);
    auto* rhs = Expr(4.5_h);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 4u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.998p+1
%3 = OpConstant %1 0x1.2p+2
%5 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareF16Test, Vector) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();

    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareF16Test,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
                    BinaryData{ast::BinaryOp::kGreaterThan, "OpFOrdGreaterThan"},
                    BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual"},
                    BinaryData{ast::BinaryOp::kLessThan, "OpFOrdLessThan"},
                    BinaryData{ast::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

TEST_F(BuilderTest, Binary_Multiply_VectorScalar_F32) {
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = OpVectorTimesScalar %1 %4 %3\n");
}

TEST_F(BuilderTest, Binary_Multiply_VectorScalar_F16) {
    Enable(builtin::Extension::kF16);

    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstantComposite %1 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = OpVectorTimesScalar %1 %4 %3\n");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarVector_F32) {
    auto* lhs = Expr(1_f);
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 1
%3 = OpTypeVector %1 3
%4 = OpConstantComposite %3 %2 %2 %2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = OpVectorTimesScalar %3 %4 %2\n");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarVector_F16) {
    Enable(builtin::Extension::kF16);

    auto* lhs = Expr(1_h);
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 5u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+0
%3 = OpTypeVector %1 3
%4 = OpConstantComposite %3 %2 %2 %2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              "%5 = OpVectorTimesScalar %3 %4 %2\n");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixScalar_F32) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr(1_f));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpConstant %5 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%9 = OpMatrixTimesScalar %3 %7 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixScalar_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("mat", ty.mat3x3<f16>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr(1_h));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 16
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpConstant %5 0x1p+0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%9 = OpMatrixTimesScalar %3 %7 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarMatrix_F32) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr(1_f), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpConstant %5 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%8 = OpLoad %3 %1
%9 = OpMatrixTimesScalar %3 %8 %7
)");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarMatrix_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("mat", ty.mat3x3<f16>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr(1_h), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 16
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpConstant %5 0x1p+0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%8 = OpLoad %3 %1
%9 = OpMatrixTimesScalar %3 %8 %7
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixVector_F32) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), rhs);

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpConstant %5 1
%9 = OpConstantComposite %4 %8 %8 %8
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%10 = OpMatrixTimesVector %4 %7 %9
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixVector_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("mat", ty.mat3x3<f16>());
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), rhs);

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 16
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpConstant %5 0x1p+0
%9 = OpConstantComposite %4 %8 %8 %8
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%10 = OpMatrixTimesVector %4 %7 %9
)");
}

TEST_F(BuilderTest, Binary_Multiply_VectorMatrix_F32) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpConstant %5 1
%8 = OpConstantComposite %4 %7 %7 %7
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%10 = OpVectorTimesMatrix %4 %8 %9
)");
}

TEST_F(BuilderTest, Binary_Multiply_VectorMatrix_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("mat", ty.mat3x3<f16>());
    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 16
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpConstant %5 0x1p+0
%8 = OpConstantComposite %4 %7 %7 %7
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%10 = OpVectorTimesMatrix %4 %8 %9
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixMatrix_F32) {
    auto* var = Var("mat", ty.mat3x3<f32>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%9 = OpMatrixTimesMatrix %3 %7 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixMatrix_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("mat", ty.mat3x3<f16>());
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), Expr("mat"));

    WrapInFunction(var, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 9u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%5 = OpTypeFloat 16
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%9 = OpMatrixTimesMatrix %3 %7 %8
)");
}

TEST_F(BuilderTest, Binary_LogicalAnd) {
    auto* v0 = Var("a", Expr(1_i));
    auto* v1 = Var("b", Expr(2_i));
    auto* v2 = Var("c", Expr(3_i));
    auto* v3 = Var("d", Expr(4_i));
    auto* expr = LogicalAnd(Equal("a", "b"), Equal("c", "d"));

    WrapInFunction(v0, v1, v2, v3, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    b.GenerateLabel(b.Module().NextId());
    ASSERT_TRUE(b.GenerateFunctionVariable(v0)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v1)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v2)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v3)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 22u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
%5 = OpTypePointer Function %2
%6 = OpConstantNull %2
%7 = OpConstant %2 2
%9 = OpConstant %2 3
%11 = OpConstant %2 4
%16 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpLabel
OpStore %4 %3
OpStore %8 %7
OpStore %10 %9
OpStore %12 %11
%13 = OpLoad %2 %4
%14 = OpLoad %2 %8
%15 = OpIEqual %16 %13 %14
OpSelectionMerge %17 None
OpBranchConditional %15 %18 %17
%18 = OpLabel
%19 = OpLoad %2 %10
%20 = OpLoad %2 %12
%21 = OpIEqual %16 %19 %20
OpBranch %17
%17 = OpLabel
%22 = OpPhi %16 %15 %1 %21 %18
)");
}

TEST_F(BuilderTest, Binary_LogicalAnd_WithLoads) {
    auto* a_var = GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, Expr(true));
    auto* b_var = GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate, Expr(false));
    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    b.GenerateLabel(b.Module().NextId());

    ASSERT_TRUE(b.GenerateGlobalVariable(a_var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(b_var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%5 = OpTypePointer Private %2
%4 = OpVariable %5 Private %3
%6 = OpConstantNull %2
%7 = OpVariable %5 Private %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

    auto* t = Let("t", Expr(true));
    auto* f = Let("f", Expr(false));

    auto* logical_and_expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr(t), Expr(f));

    auto* expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr(t), logical_and_expr);

    WrapInFunction(t, f, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(t)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(f)) << b.Diagnostics();
    b.GenerateLabel(b.Module().NextId());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %2 %5 %6
%6 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %2 %8 %7
%8 = OpLabel
OpBranch %7
%7 = OpLabel
%9 = OpPhi %1 %2 %6 %3 %8
OpBranch %5
%5 = OpLabel
%10 = OpPhi %1 %2 %4 %9 %7
)");
}

TEST_F(BuilderTest, Binary_logicalAnd_Nested_LogicalOr) {
    // Test an expression like
    //    a && (b || c)
    // From: crbug.com/tint/355

    auto* t = Let("t", Expr(true));
    auto* f = Let("f", Expr(false));

    auto* logical_or_expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr(t), Expr(f));

    auto* expr =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr(t), logical_or_expr);

    WrapInFunction(t, f, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(t)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(f)) << b.Diagnostics();
    b.GenerateLabel(b.Module().NextId());

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 10u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %2 %6 %5
%6 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %2 %7 %8
%8 = OpLabel
OpBranch %7
%7 = OpLabel
%9 = OpPhi %1 %2 %6 %3 %8
OpBranch %5
%5 = OpLabel
%10 = OpPhi %1 %2 %4 %9 %7
)");
}

TEST_F(BuilderTest, Binary_LogicalOr) {
    auto* v0 = Var("a", Expr(1_i));
    auto* v1 = Var("b", Expr(2_i));
    auto* v2 = Var("c", Expr(3_i));
    auto* v3 = Var("d", Expr(4_i));
    auto* expr = LogicalOr(Equal("a", "b"), Equal("c", "d"));

    WrapInFunction(v0, v1, v2, v3, expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    b.GenerateLabel(b.Module().NextId());
    ASSERT_TRUE(b.GenerateFunctionVariable(v0)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v1)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v2)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunctionVariable(v3)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 22u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
%5 = OpTypePointer Function %2
%6 = OpConstantNull %2
%7 = OpConstant %2 2
%9 = OpConstant %2 3
%11 = OpConstant %2 4
%16 = OpTypeBool
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%1 = OpLabel
OpStore %4 %3
OpStore %8 %7
OpStore %10 %9
OpStore %12 %11
%13 = OpLoad %2 %4
%14 = OpLoad %2 %8
%15 = OpIEqual %16 %13 %14
OpSelectionMerge %17 None
OpBranchConditional %15 %17 %18
%18 = OpLabel
%19 = OpLoad %2 %10
%20 = OpLoad %2 %12
%21 = OpIEqual %16 %19 %20
OpBranch %17
%17 = OpLabel
%22 = OpPhi %16 %15 %1 %21 %18
)");
}

TEST_F(BuilderTest, Binary_LogicalOr_WithLoads) {
    auto* a_var = GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, Expr(true));
    auto* b_var = GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate, Expr(false));

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));

    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    b.GenerateLabel(b.Module().NextId());

    ASSERT_TRUE(b.GenerateGlobalVariable(a_var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(b_var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateBinaryExpression(expr), 12u) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
%5 = OpTypePointer Private %2
%4 = OpVariable %5 Private %3
%6 = OpConstantNull %2
%7 = OpVariable %5 Private %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

enum class Type { f32, f16, i32, u32 };
static const ast::Expression* MakeVectorExpr(ProgramBuilder* builder, Type type) {
    auto name = builder->Symbols().New();
    switch (type) {
        case Type::f32:
            builder->GlobalVar(name, builder->ty.vec3<f32>(), builder->vec3<f32>(1_f, 1_f, 1_f),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::f16:
            builder->GlobalVar(name, builder->ty.vec3<f16>(), builder->vec3<f16>(1_h, 1_h, 1_h),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::i32:
            builder->GlobalVar(name, builder->ty.vec3<i32>(), builder->vec3<i32>(1_i, 1_i, 1_i),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::u32:
            builder->GlobalVar(name, builder->ty.vec3<u32>(), builder->vec3<u32>(1_u, 1_u, 1_u),
                               builtin::AddressSpace::kPrivate);
            break;
    }
    return builder->Expr(name);
}
static const ast::Expression* MakeScalarExpr(ProgramBuilder* builder, Type type) {
    auto name = builder->Symbols().New();
    switch (type) {
        case Type::f32:
            builder->GlobalVar(name, builder->ty.f32(), builder->Expr(1_f),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::f16:
            builder->GlobalVar(name, builder->ty.f16(), builder->Expr(1_h),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::i32:
            builder->GlobalVar(name, builder->ty.i32(), builder->Expr(1_i),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::u32:
            builder->GlobalVar(name, builder->ty.u32(), builder->Expr(1_u),
                               builtin::AddressSpace::kPrivate);
            break;
    }
    return builder->Expr(name);
}
static std::string OpTypeDecl(Type type) {
    switch (type) {
        case Type::f32:
            return "OpTypeFloat 32";
        case Type::f16:
            return "OpTypeFloat 16";
        case Type::i32:
            return "OpTypeInt 32 1";
        case Type::u32:
            return "OpTypeInt 32 0";
    }
    return {};
}
static std::string ConstantValue(Type type) {
    switch (type) {
        case Type::f32:
        case Type::i32:
        case Type::u32:
            return "1";
        case Type::f16:
            return "0x1p+0";
    }
    return {};
}
static std::string CapabilityDecl(Type type) {
    switch (type) {
        case Type::f32:
        case Type::i32:
        case Type::u32:
            return "OpCapability Shader";
        case Type::f16:
            return R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16)";
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

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeVectorExpr(this, param.type);
    const ast::Expression* rhs = MakeScalarExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string constant_value = ConstantValue(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "test_function"
OpExecutionMode %11 LocalSize 1 1 1
OpName %5 "tint_symbol"
OpName %7 "tint_symbol_1"
OpName %11 "test_function"
%2 = )" + op_type_decl + R"(
%1 = OpTypeVector %2 3
%3 = OpConstant %2 )" + constant_value +
                                  R"(
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypePointer Private %2
%7 = OpVariable %8 Private %3
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%17 = OpTypePointer Function %1
%18 = OpConstantNull %1
%11 = OpFunction %10 None %9
%12 = OpLabel
%16 = OpVariable %17 Function %18
%13 = OpLoad %1 %5
%14 = OpLoad %2 %7
%19 = OpCompositeConstruct %1 %14 %14 %14
%15 = )" + param.name + R"( %1 %13 %19
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
TEST_P(BinaryArithVectorScalarTest, ScalarVector) {
    auto& param = GetParam();

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeScalarExpr(this, param.type);
    const ast::Expression* rhs = MakeVectorExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string constant_value = ConstantValue(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "test_function"
OpExecutionMode %11 LocalSize 1 1 1
OpName %3 "tint_symbol"
OpName %7 "tint_symbol_1"
OpName %11 "test_function"
%1 = )" + op_type_decl + R"(
%2 = OpConstant %1 )" + constant_value +
                                  R"(
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%5 = OpTypeVector %1 3
%6 = OpConstantComposite %5 %2 %2 %2
%8 = OpTypePointer Private %5
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%17 = OpTypePointer Function %5
%18 = OpConstantNull %5
%11 = OpFunction %10 None %9
%12 = OpLabel
%16 = OpVariable %17 Function %18
%13 = OpLoad %1 %3
%14 = OpLoad %5 %7
%19 = OpCompositeConstruct %5 %13 %13 %13
%15 = )" + param.name + R"( %5 %19 %14
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

                                         Param{Type::f16, ast::BinaryOp::kAdd, "OpFAdd"},
                                         Param{Type::f16, ast::BinaryOp::kDivide, "OpFDiv"},
                                         Param{Type::f16, ast::BinaryOp::kSubtract, "OpFSub"},

                                         Param{Type::i32, ast::BinaryOp::kAdd, "OpIAdd"},
                                         Param{Type::i32, ast::BinaryOp::kDivide, "OpSDiv"},
                                         Param{Type::i32, ast::BinaryOp::kModulo, "OpSRem"},
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

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeVectorExpr(this, param.type);
    const ast::Expression* rhs = MakeScalarExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string constant_value = ConstantValue(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "test_function"
OpExecutionMode %11 LocalSize 1 1 1
OpName %5 "tint_symbol"
OpName %7 "tint_symbol_1"
OpName %11 "test_function"
%2 = )" + op_type_decl + R"(
%1 = OpTypeVector %2 3
%3 = OpConstant %2 )" + constant_value +
                                  R"(
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypePointer Private %2
%7 = OpVariable %8 Private %3
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%11 = OpFunction %10 None %9
%12 = OpLabel
%13 = OpLoad %1 %5
%14 = OpLoad %2 %7
%15 = OpVectorTimesScalar %1 %13 %14
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
TEST_P(BinaryArithVectorScalarMultiplyTest, ScalarVector) {
    auto& param = GetParam();

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeScalarExpr(this, param.type);
    const ast::Expression* rhs = MakeVectorExpr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string constant_value = ConstantValue(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "test_function"
OpExecutionMode %11 LocalSize 1 1 1
OpName %3 "tint_symbol"
OpName %7 "tint_symbol_1"
OpName %11 "test_function"
%1 = )" + op_type_decl + R"(
%2 = OpConstant %1 )" + constant_value +
                                  R"(
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%5 = OpTypeVector %1 3
%6 = OpConstantComposite %5 %2 %2 %2
%8 = OpTypePointer Private %5
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%11 = OpFunction %10 None %9
%12 = OpLabel
%13 = OpLoad %1 %3
%14 = OpLoad %5 %7
%15 = OpVectorTimesScalar %5 %14 %13
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithVectorScalarMultiplyTest,
                         testing::Values(Param{Type::f32, ast::BinaryOp::kMultiply, "OpFMul"},
                                         Param{Type::f16, ast::BinaryOp::kMultiply, "OpFMul"}));

}  // namespace BinaryArithVectorScalar

namespace BinaryArithMatrixMatrix {

enum class Type { f32, f16 };
static const ast::Expression* MakeMat3x4Expr(ProgramBuilder* builder, Type type) {
    auto name = builder->Symbols().New();
    switch (type) {
        case Type::f32:
            builder->GlobalVar(name, builder->ty.mat3x4<f32>(), builder->mat3x4<f32>(),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::f16:
            builder->GlobalVar(name, builder->ty.mat3x4<f16>(), builder->mat3x4<f16>(),
                               builtin::AddressSpace::kPrivate);
            break;
    }
    return builder->Expr(name);
}
static const ast::Expression* MakeMat4x3Expr(ProgramBuilder* builder, Type type) {
    auto name = builder->Symbols().New();
    switch (type) {
        case Type::f32:
            builder->GlobalVar(name, builder->ty.mat4x3<f32>(), builder->mat4x3<f32>(),
                               builtin::AddressSpace::kPrivate);
            break;
        case Type::f16:
            builder->GlobalVar(name, builder->ty.mat4x3<f16>(), builder->mat4x3<f16>(),
                               builtin::AddressSpace::kPrivate);
    }
    return builder->Expr(name);
}
static std::string OpTypeDecl(Type type) {
    switch (type) {
        case Type::f32:
            return "OpTypeFloat 32";
        case Type::f16:
            return "OpTypeFloat 16";
    }
    return {};
}
static std::string CapabilityDecl(Type type) {
    switch (type) {
        case Type::f32:
            return "OpCapability Shader";
        case Type::f16:
            return R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16)";
    }
    return {};
}

struct Param {
    Type type;
    ast::BinaryOp op;
    std::string name;
};

using BinaryArithMatrixMatrix = TestParamHelper<Param>;
TEST_P(BinaryArithMatrixMatrix, AddOrSubtract) {
    auto& param = GetParam();

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeMat3x4Expr(this, param.type);
    const ast::Expression* rhs = MakeMat3x4Expr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %10 "test_function"
OpExecutionMode %10 LocalSize 1 1 1
OpName %5 "tint_symbol"
OpName %7 "tint_symbol_1"
OpName %10 "test_function"
%3 = )" + op_type_decl + R"(
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstantNull %1
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%7 = OpVariable %6 Private %4
%9 = OpTypeVoid
%8 = OpTypeFunction %9
%10 = OpFunction %9 None %8
%11 = OpLabel
%12 = OpLoad %1 %5
%13 = OpLoad %1 %7
%15 = OpCompositeExtract %2 %12 0
%16 = OpCompositeExtract %2 %13 0
%17 = )" + param.name + R"( %2 %15 %16
%18 = OpCompositeExtract %2 %12 1
%19 = OpCompositeExtract %2 %13 1
%20 = )" + param.name + R"( %2 %18 %19
%21 = OpCompositeExtract %2 %12 2
%22 = OpCompositeExtract %2 %13 2
%23 = )" + param.name + R"( %2 %21 %22
%24 = OpCompositeConstruct %1 %17 %20 %23
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(  //
    BuilderTest,
    BinaryArithMatrixMatrix,
    testing::Values(Param{Type::f32, ast::BinaryOp::kAdd, "OpFAdd"},
                    Param{Type::f32, ast::BinaryOp::kSubtract, "OpFSub"},
                    Param{Type::f16, ast::BinaryOp::kAdd, "OpFAdd"},
                    Param{Type::f16, ast::BinaryOp::kSubtract, "OpFSub"}));

using BinaryArithMatrixMatrixMultiply = TestParamHelper<Param>;
TEST_P(BinaryArithMatrixMatrixMultiply, Multiply) {
    auto& param = GetParam();

    if (param.type == Type::f16) {
        Enable(builtin::Extension::kF16);
    }

    const ast::Expression* lhs = MakeMat3x4Expr(this, param.type);
    const ast::Expression* rhs = MakeMat4x3Expr(this, param.type);
    std::string op_type_decl = OpTypeDecl(param.type);
    std::string capability_decl = CapabilityDecl(param.type);

    auto* expr = create<ast::BinaryExpression>(param.op, lhs, rhs);

    WrapInFunction(expr);

    spirv::Builder& b = Build();
    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), capability_decl + R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %14 "test_function"
OpExecutionMode %14 LocalSize 1 1 1
OpName %5 "tint_symbol"
OpName %10 "tint_symbol_1"
OpName %14 "test_function"
%3 = )" + op_type_decl + R"(
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstantNull %1
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVector %3 3
%7 = OpTypeMatrix %8 4
%9 = OpConstantNull %7
%11 = OpTypePointer Private %7
%10 = OpVariable %11 Private %9
%13 = OpTypeVoid
%12 = OpTypeFunction %13
%19 = OpTypeMatrix %2 4
%14 = OpFunction %13 None %12
%15 = OpLabel
%16 = OpLoad %1 %5
%17 = OpLoad %7 %10
%18 = OpMatrixTimesMatrix %19 %16 %17
OpReturn
OpFunctionEnd
)");

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(  //
    BuilderTest,
    BinaryArithMatrixMatrixMultiply,
    testing::Values(Param{Type::f32, ast::BinaryOp::kMultiply, ""},
                    Param{Type::f16, ast::BinaryOp::kMultiply, ""}));

}  // namespace BinaryArithMatrixMatrix

}  // namespace
}  // namespace tint::writer::spirv
