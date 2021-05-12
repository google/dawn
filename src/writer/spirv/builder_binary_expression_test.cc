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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
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
  auto* expr =
      create<ast::BinaryExpression>(param.op, Expr("param"), Expr("param"));

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
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
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
                    BinaryData{ast::BinaryOp::kShiftRight,
                               "OpShiftRightLogical"},
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
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryArithFloatTest,
    testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpFAdd"},
                    BinaryData{ast::BinaryOp::kDivide, "OpFDiv"},
                    BinaryData{ast::BinaryOp::kModulo, "OpFMod"},
                    BinaryData{ast::BinaryOp::kMultiply, "OpFMul"},
                    BinaryData{ast::BinaryOp::kSubtract, "OpFSub"}));

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
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
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
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
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
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
        BinaryData{ast::BinaryOp::kGreaterThan, "OpFOrdGreaterThan"},
        BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual"},
        BinaryData{ast::BinaryOp::kLessThan, "OpFOrdLessThan"},
        BinaryData{ast::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual"},
        BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

TEST_F(BuilderTest, Binary_Multiply_VectorScalar) {
  auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
  auto* rhs = Expr(1.f);

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

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

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

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

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply,
                                             Expr("mat"), Expr(1.f));

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

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply,
                                             Expr(1.f), Expr("mat"));

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

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("mat"), rhs);

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

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, Expr("mat"));

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

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply,
                                             Expr("mat"), Expr("mat"));

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
  auto* lhs =
      create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(1), Expr(2));

  auto* rhs =
      create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(3), Expr(4));

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, lhs, rhs);

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
  auto* a_var =
      Global("a", ty.bool_(), ast::StorageClass::kPrivate, Expr(true));
  auto* b_var =
      Global("b", ty.bool_(), ast::StorageClass::kPrivate, Expr(false));

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                             Expr("a"), Expr("b"));

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

  auto* logical_and_expr = create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalAnd, Expr(true), Expr(false));

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                             Expr(true), logical_and_expr);

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

  auto* logical_or_expr = create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr, Expr(true), Expr(false));

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                             Expr(true), logical_or_expr);

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
  auto* lhs =
      create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(1), Expr(2));

  auto* rhs =
      create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(3), Expr(4));

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, lhs, rhs);

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
  auto* a_var =
      Global("a", ty.bool_(), ast::StorageClass::kPrivate, Expr(true));
  auto* b_var =
      Global("b", ty.bool_(), ast::StorageClass::kPrivate, Expr(false));

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                             Expr("a"), Expr("b"));

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

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
