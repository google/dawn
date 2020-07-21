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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

struct IntrinsicData {
  std::string name;
  std::string op;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicBoolTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicBoolTest, Call_Bool) {
  auto param = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&bool_type, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeBool
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %4 %7\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicBoolTest,
                         testing::Values(IntrinsicData{"any", "OpAny"},
                                         IntrinsicData{"all", "OpAll"}));

using IntrinsicFloatTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicFloatTest, Call_Float_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = )" + param.op +
                " %6 %7\n");
}

TEST_P(IntrinsicFloatTest, Call_Float_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeBool
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = )" + param.op +
                " %7 %9\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"is_nan", "OpIsNan"},
                                         IntrinsicData{"is_inf", "OpIsInf"}));

TEST_F(BuilderTest, Call_Dot) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("dot"),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%6 = OpDot %4 %7 %8
)");
}

using IntrinsicDeriveTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicDeriveTest, Call_Derivative_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();

  if (param.name != "dpdx" && param.name != "dpdy" && param.name != "fwidth") {
    EXPECT_EQ(DumpInstructions(b.capabilities()),
              R"(OpCapability DerivativeControl
)");
  }

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    IntrinsicDeriveTest,
    testing::Values(IntrinsicData{"dpdx", "OpDPdx"},
                    IntrinsicData{"dpdx_fine", "OpDPdxFine"},
                    IntrinsicData{"dpdx_coarse", "OpDPdxCoarse"},
                    IntrinsicData{"dpdy", "OpDPdy"},
                    IntrinsicData{"dpdy_fine", "OpDPdyFine"},
                    IntrinsicData{"dpdy_coarse", "OpDPdyCoarse"},
                    IntrinsicData{"fwidth", "OpFwidth"},
                    IntrinsicData{"fwidth_fine", "OpFwidthFine"},
                    IntrinsicData{"fwidth_coarse", "OpFwidthCoarse"}));

TEST_F(BuilderTest, Call_OuterProduct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat(&f32, 2, 3);

  auto v2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kPrivate, &vec2);
  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outer_product"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v2.get());
  td.RegisterVariableForTesting(v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v2.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 10u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeVector %4 3
%7 = OpTypePointer Private %8
%9 = OpConstantNull %8
%6 = OpVariable %7 Private %9
%11 = OpTypeMatrix %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %8 %6
%10 = OpOuterProduct %11 %12 %13
)");
}

TEST_F(BuilderTest, Call_Select) {
  ast::type::F32Type f32;
  ast::type::BoolType bool_type;
  ast::type::VectorType bool_vec3(&bool_type, 3);
  ast::type::VectorType vec3(&f32, 3);

  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);
  auto bool_v3 = std::make_unique<ast::Variable>(
      "bool_v3", ast::StorageClass::kPrivate, &bool_vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("bool_v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"), std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v3.get());
  td.RegisterVariableForTesting(bool_v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%9 = OpTypeBool
%8 = OpTypeVector %9 3
%7 = OpTypePointer Private %8
%10 = OpConstantNull %8
%6 = OpVariable %7 Private %10
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %3 %1
%14 = OpLoad %8 %6
%11 = OpSelect %3 %12 %13 %14
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
