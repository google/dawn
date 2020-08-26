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
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Import = TestHelper;

struct HlslImportData {
  const char* name;
  const char* hlsl_name;
};
inline std::ostream& operator<<(std::ostream& out, HlslImportData data) {
  out << data.name;
  return out;
}

using HlslImportData_SingleParamTest =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_SingleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Import,
    HlslImportData_SingleParamTest,
    testing::Values(HlslImportData{"acos", "acos"},
                    HlslImportData{"asin", "asin"},
                    HlslImportData{"atan", "atan"},
                    HlslImportData{"cos", "cos"},
                    HlslImportData{"cosh", "cosh"},
                    HlslImportData{"ceil", "ceil"},
                    HlslImportData{"degrees", "degrees"},
                    HlslImportData{"exp", "exp"},
                    HlslImportData{"exp2", "exp2"},
                    HlslImportData{"fabs", "abs"},
                    HlslImportData{"floor", "floor"},
                    HlslImportData{"fract", "frac"},
                    HlslImportData{"interpolateatcentroid",
                                   "EvaluateAttributeAtCentroid"},
                    HlslImportData{"inversesqrt", "rsqrt"},
                    HlslImportData{"length", "length"},
                    HlslImportData{"log", "log"},
                    HlslImportData{"log2", "log2"},
                    HlslImportData{"normalize", "normalize"},
                    HlslImportData{"radians", "radians"},
                    HlslImportData{"round", "round"},
                    HlslImportData{"fsign", "sign"},
                    HlslImportData{"sin", "sin"},
                    HlslImportData{"sinh", "sinh"},
                    HlslImportData{"sqrt", "sqrt"},
                    HlslImportData{"tan", "tan"},
                    HlslImportData{"tanh", "tanh"},
                    HlslImportData{"trunc", "trunc"}));

TEST_F(HlslGeneratorImplTest_Import, DISABLED_HlslImportData_Acosh) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Import, DISABLED_HlslImportData_ASinh) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Import, DISABLED_HlslImportData_ATanh) {
  FAIL();
}

using HlslImportData_SingleIntParamTest =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_SingleIntParamTest, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", param.name}),
                           std::move(params));

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_SingleIntParamTest,
                         testing::Values(HlslImportData{"sabs", "abs"},
                                         HlslImportData{"ssign", "sign"}));

using HlslImportData_DualParamTest =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_DualParamTest, FloatScalar) {
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

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(),
            std::string(param.hlsl_name) + "(1.00000000f, 2.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParamTest,
                         testing::Values(HlslImportData{"atan2", "atan2"},
                                         HlslImportData{"distance", "distance"},
                                         HlslImportData{"fmax", "max"},
                                         HlslImportData{"fmin", "min"},
                                         HlslImportData{"nmax", "max"},
                                         HlslImportData{"nmin", "min"},
                                         HlslImportData{"pow", "pow"},
                                         HlslImportData{"reflect", "reflect"},
                                         HlslImportData{"step", "step"}));

using HlslImportData_DualParam_VectorTest =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_DualParam_VectorTest, FloatVector) {
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

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(),
            std::string(param.hlsl_name) +
                "(vector<float, 3>(1.00000000f, 2.00000000f, 3.00000000f), "
                "vector<float, 3>(4.00000000f, 5.00000000f, 6.00000000f))");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParam_VectorTest,
                         testing::Values(HlslImportData{"cross", "cross"}));

using HlslImportData_DualParam_Int_Test =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_DualParam_Int_Test, IntScalar) {
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

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParam_Int_Test,
                         testing::Values(HlslImportData{"smax", "max"},
                                         HlslImportData{"smin", "min"},
                                         HlslImportData{"umax", "max"},
                                         HlslImportData{"umin", "min"}));

using HlslImportData_TripleParamTest =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_TripleParamTest, FloatScalar) {
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

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) +
                          "(1.00000000f, 2.00000000f, 3.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Import,
    HlslImportData_TripleParamTest,
    testing::Values(HlslImportData{"faceforward", "faceforward"},
                    HlslImportData{"fma", "fma"},
                    HlslImportData{"fclamp", "clamp"},
                    HlslImportData{"nclamp", "clamp"},
                    HlslImportData{"smoothstep", "smoothstep"}));

TEST_F(HlslGeneratorImplTest_Import, DISABLED_HlslImportData_FMix) {
  FAIL();
}

using HlslImportData_TripleParam_Int_Test =
    TestHelperBase<testing::TestWithParam<HlslImportData>>;
TEST_P(HlslImportData_TripleParam_Int_Test, IntScalar) {
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

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_TripleParam_Int_Test,
                         testing::Values(HlslImportData{"sclamp", "clamp"},
                                         HlslImportData{"uclamp", "clamp"}));

TEST_F(HlslGeneratorImplTest_Import, HlslImportData_Determinant) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "determinant"}),
                           std::move(params));

  mod()->AddGlobalVariable(std::move(var));
  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  // Register the global
  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitImportFunction(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), std::string("determinant(var)"));
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
