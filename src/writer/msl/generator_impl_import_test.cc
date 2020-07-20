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

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

struct MslImportData {
  const char* name;
  const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, MslImportData data) {
  out << data.name;
  return out;
}
using MslImportData_SingleParamTest = testing::TestWithParam<MslImportData>;
TEST_P(MslImportData_SingleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(),
            std::string("metal::") + param.msl_name + "(1.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_SingleParamTest,
                         testing::Values(MslImportData{"acos", "acos"},
                                         MslImportData{"acosh", "acosh"},
                                         MslImportData{"asin", "asin"},
                                         MslImportData{"asinh", "asinh"},
                                         MslImportData{"atan", "atan"},
                                         MslImportData{"atanh", "atanh"},
                                         MslImportData{"cos", "cos"},
                                         MslImportData{"cosh", "cosh"},
                                         MslImportData{"ceil", "ceil"},
                                         MslImportData{"exp", "exp"},
                                         MslImportData{"exp2", "exp2"},
                                         MslImportData{"fabs", "fabs"},
                                         MslImportData{"floor", "floor"},
                                         MslImportData{"fract", "fract"},
                                         MslImportData{"inversesqrt", "rsqrt"},
                                         MslImportData{"length", "length"},
                                         MslImportData{"log", "log"},
                                         MslImportData{"log2", "log2"},
                                         MslImportData{"normalize",
                                                       "normalize"},
                                         MslImportData{"round", "round"},
                                         MslImportData{"fsign", "sign"},
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

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "sabs"}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), R"(metal::abs(1))");
}

using MslImportData_DualParamTest = testing::TestWithParam<MslImportData>;
TEST_P(MslImportData_DualParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::") + param.msl_name +
                            "(1.00000000f, 2.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParamTest,
                         testing::Values(MslImportData{"atan2", "atan2"},
                                         MslImportData{"distance", "distance"},
                                         MslImportData{"fmax", "fmax"},
                                         MslImportData{"fmin", "fmin"},
                                         MslImportData{"nmax", "fmax"},
                                         MslImportData{"nmin", "fmin"},
                                         MslImportData{"pow", "pow"},
                                         MslImportData{"reflect", "reflect"},
                                         MslImportData{"step", "step"}));

using MslImportData_DualParam_VectorTest =
    testing::TestWithParam<MslImportData>;
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

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::") + param.msl_name +
                            "(float3(1.00000000f, 2.00000000f, 3.00000000f), "
                            "float3(4.00000000f, 5.00000000f, 6.00000000f))");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_VectorTest,
                         testing::Values(MslImportData{"cross", "cross"}));

using MslImportData_DualParam_Int_Test = testing::TestWithParam<MslImportData>;
TEST_P(MslImportData_DualParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::") + param.msl_name + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_Int_Test,
                         testing::Values(MslImportData{"smax", "max"},
                                         MslImportData{"smin", "min"},
                                         MslImportData{"umax", "max"},
                                         MslImportData{"umin", "min"}));

using MslImportData_TripleParamTest = testing::TestWithParam<MslImportData>;
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

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::") + param.msl_name +
                            "(1.00000000f, 2.00000000f, 3.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslImportData_TripleParamTest,
    testing::Values(MslImportData{"faceforward", "faceforward"},
                    MslImportData{"fma", "fma"},
                    MslImportData{"fclamp", "clamp"},
                    MslImportData{"fmix", "mix"},
                    MslImportData{"nclamp", "clamp"},
                    MslImportData{"smoothstep", "smoothstep"}));

using MslImportData_TripleParam_Int_Test =
    testing::TestWithParam<MslImportData>;
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

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::") + param.msl_name + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_TripleParam_Int_Test,
                         testing::Values(MslImportData{"sclamp", "clamp"},
                                         MslImportData{"uclamp", "clamp"}));

TEST_F(MslGeneratorImplTest, MslImportData_Determinant) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "determinant"}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  mod.AddGlobalVariable(std::move(var));
  mod.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  TypeDeterminer td(&ctx, &mod);
  // Register the global
  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitImportFunction(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("metal::determinant(var)"));
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
