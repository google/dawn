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

using HlslImportData_SingleParamTest = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_SingleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(param.name);
  ast::CallExpression expr(ident, params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_SingleParamTest,
                         testing::Values(HlslImportData{"abs", "abs"},
                                         HlslImportData{"acos", "acos"},
                                         HlslImportData{"asin", "asin"},
                                         HlslImportData{"atan", "atan"},
                                         HlslImportData{"cos", "cos"},
                                         HlslImportData{"cosh", "cosh"},
                                         HlslImportData{"ceil", "ceil"},
                                         HlslImportData{"exp", "exp"},
                                         HlslImportData{"exp2", "exp2"},
                                         HlslImportData{"floor", "floor"},
                                         HlslImportData{"fract", "frac"},
                                         HlslImportData{"inverseSqrt", "rsqrt"},
                                         HlslImportData{"length", "length"},
                                         HlslImportData{"log", "log"},
                                         HlslImportData{"log2", "log2"},
                                         HlslImportData{"normalize",
                                                        "normalize"},
                                         HlslImportData{"round", "round"},
                                         HlslImportData{"sign", "sign"},
                                         HlslImportData{"sin", "sin"},
                                         HlslImportData{"sinh", "sinh"},
                                         HlslImportData{"sqrt", "sqrt"},
                                         HlslImportData{"tan", "tan"},
                                         HlslImportData{"tanh", "tanh"},
                                         HlslImportData{"trunc", "trunc"}));

using HlslImportData_SingleIntParamTest = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_SingleIntParamTest, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_SingleIntParamTest,
                         testing::Values(HlslImportData{"abs", "abs"}));

using HlslImportData_DualParamTest = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_DualParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.f)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            std::string(param.hlsl_name) + "(1.00000000f, 2.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParamTest,
                         testing::Values(HlslImportData{"atan2", "atan2"},
                                         HlslImportData{"distance", "distance"},
                                         HlslImportData{"max", "max"},
                                         HlslImportData{"min", "min"},
                                         HlslImportData{"pow", "pow"},
                                         HlslImportData{"reflect", "reflect"},
                                         HlslImportData{"step", "step"}));

using HlslImportData_DualParam_VectorTest = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_DualParam_VectorTest, FloatVector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(
      &vec, ast::ExpressionList{
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 1.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 2.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 3.f)),
            }));

  params.push_back(create<ast::TypeConstructorExpression>(
      &vec, ast::ExpressionList{
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 4.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 5.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(&f32, 6.f)),
            }));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            std::string(param.hlsl_name) +
                "(vector<float, 3>(1.00000000f, 2.00000000f, 3.00000000f), "
                "vector<float, 3>(4.00000000f, 5.00000000f, 6.00000000f))");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParam_VectorTest,
                         testing::Values(HlslImportData{"cross", "cross"}));

using HlslImportData_DualParam_Int_Test = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_DualParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_DualParam_Int_Test,
                         testing::Values(HlslImportData{"max", "max"},
                                         HlslImportData{"min", "min"}));

using HlslImportData_TripleParamTest = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_TripleParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.f)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) +
                          "(1.00000000f, 2.00000000f, 3.00000000f)");
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Import,
    HlslImportData_TripleParamTest,
    testing::Values(HlslImportData{"faceForward", "faceforward"},
                    HlslImportData{"fma", "fma"},
                    HlslImportData{"clamp", "clamp"},
                    HlslImportData{"smoothStep", "smoothstep"}));

TEST_F(HlslGeneratorImplTest_Import, DISABLED_HlslImportData_FMix) {
  FAIL();
}

using HlslImportData_TripleParam_Int_Test = TestParamHelper<HlslImportData>;
TEST_P(HlslImportData_TripleParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string(param.hlsl_name) + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Import,
                         HlslImportData_TripleParam_Int_Test,
                         testing::Values(HlslImportData{"clamp", "clamp"}));

TEST_F(HlslGeneratorImplTest_Import, HlslImportData_Determinant) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto* var = create<ast::Variable>("var", ast::StorageClass::kFunction, &mat);

  ast::ExpressionList params;
  params.push_back(create<ast::IdentifierExpression>("var"));

  ast::CallExpression expr(create<ast::IdentifierExpression>("determinant"),
                           params);

  mod.AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), std::string("determinant(var)"));
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
