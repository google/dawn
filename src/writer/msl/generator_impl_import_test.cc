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
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

struct MslImportData {
  const char* name;
  const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, MslImportData data) {
  out << data.name;
  return out;
}
using MslImportData_SingleParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_SingleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  auto ident = std::make_unique<ast::IdentifierExpression>(param.name);
  auto* ident_ptr = ident.get();

  ast::CallExpression call(std::move(ident), std::move(params));

  // The call type determination will set the intrinsic data for the ident
  ASSERT_TRUE(td.DetermineResultType(&call)) << td.error();

  ASSERT_EQ(gen.generate_builtin_name(ident_ptr),
            std::string("metal::") + param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_SingleParamTest,
                         testing::Values(MslImportData{"abs", "fabs"},
                                         MslImportData{"acos", "acos"},
                                         MslImportData{"asin", "asin"},
                                         MslImportData{"atan", "atan"},
                                         MslImportData{"ceil", "ceil"},
                                         MslImportData{"cos", "cos"},
                                         MslImportData{"cosh", "cosh"},
                                         MslImportData{"exp", "exp"},
                                         MslImportData{"exp2", "exp2"},
                                         MslImportData{"floor", "floor"},
                                         MslImportData{"fract", "fract"},
                                         MslImportData{"inverseSqrt", "rsqrt"},
                                         MslImportData{"length", "length"},
                                         MslImportData{"log", "log"},
                                         MslImportData{"log2", "log2"},
                                         MslImportData{"round", "round"},
                                         MslImportData{"sign", "sign"},
                                         MslImportData{"sin", "sin"},
                                         MslImportData{"sinh", "sinh"},
                                         MslImportData{"sqrt", "sqrt"},
                                         MslImportData{"tan", "tan"},
                                         MslImportData{"tanh", "tanh"},
                                         MslImportData{"trunc", "trunc"}));

TEST_F(MslGeneratorImplTest, MslImportData_SingleParamTest_IntScalar) {
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("abs"),
                           std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), R"(metal::abs(1))");
}

using MslImportData_DualParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::") + param.msl_name +
                              "(1.00000000f, 2.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParamTest,
                         testing::Values(MslImportData{"atan2", "atan2"},
                                         MslImportData{"distance", "distance"},
                                         MslImportData{"max", "fmax"},
                                         MslImportData{"min", "fmin"},
                                         MslImportData{"pow", "pow"},
                                         MslImportData{"reflect", "reflect"},
                                         MslImportData{"step", "step"}));

using MslImportData_DualParam_VectorTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_VectorTest, FloatVector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList type_params;
  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(type_params)));

  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.f)));
  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 5.f)));
  type_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 6.f)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(type_params)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::") + param.msl_name +
                              "(float3(1.00000000f, 2.00000000f, 3.00000000f), "
                              "float3(4.00000000f, 5.00000000f, 6.00000000f))");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_VectorTest,
                         testing::Values(MslImportData{"cross", "cross"}));

using MslImportData_DualParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::") + param.msl_name + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_Int_Test,
                         testing::Values(MslImportData{"max", "max"},
                                         MslImportData{"min", "min"}));

using MslImportData_TripleParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::") + param.msl_name +
                              "(1.00000000f, 2.00000000f, 3.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslImportData_TripleParamTest,
    testing::Values(MslImportData{"faceForward", "faceforward"},
                    MslImportData{"fma", "fma"},
                    MslImportData{"mix", "mix"},
                    MslImportData{"clamp", "clamp"},
                    MslImportData{"smoothStep", "smoothstep"}));

using MslImportData_TripleParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            std::string("metal::") + param.msl_name + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_TripleParam_Int_Test,
                         testing::Values(MslImportData{"clamp", "clamp"},
                                         MslImportData{"clamp", "clamp"}));

TEST_F(MslGeneratorImplTest, MslImportData_Determinant) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("determinant"),
      std::move(params));

  mod.AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::determinant(var)"));
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
