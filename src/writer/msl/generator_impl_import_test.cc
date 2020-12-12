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

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod.RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  // The call type determination will set the intrinsic data for the ident
  ASSERT_TRUE(td.DetermineResultType(&call)) << td.error();

  ASSERT_EQ(gen.generate_builtin_name(ident),
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
  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 1)));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod.RegisterSymbol("abs"), "abs"),
      params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), R"(metal::abs(1))");
}

using MslImportData_DualParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParamTest, FloatScalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 2.f)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod.RegisterSymbol(param.name), param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            std::string("metal::") + param.msl_name + "(1.0f, 2.0f)");
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

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList type_params;

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(
      &vec, ast::ExpressionList{
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 1.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 2.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 3.f)),
            }));

  params.push_back(create<ast::TypeConstructorExpression>(
      &vec, ast::ExpressionList{
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 4.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 5.f)),
                create<ast::ScalarConstructorExpression>(
                    create<ast::FloatLiteral>(Source{}, &f32, 6.f)),
            }));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod.RegisterSymbol(param.name), param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("metal::") + param.msl_name +
                              "(float3(1.0f, 2.0f, 3.0f), "
                              "float3(4.0f, 5.0f, 6.0f))");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_VectorTest,
                         testing::Values(MslImportData{"cross", "cross"}));

using MslImportData_DualParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_Int_Test, IntScalar) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 2)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod.RegisterSymbol(param.name), param.name),
                           params);

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

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 2.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(Source{}, &f32, 3.f)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod.RegisterSymbol(param.name), param.name),
                           params);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitCall(&expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            std::string("metal::") + param.msl_name + "(1.0f, 2.0f, 3.0f)");
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

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 2)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(Source{}, &i32, 3)));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod.RegisterSymbol(param.name), param.name),
                           params);

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
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::ExpressionList params;
  params.push_back(
      create<ast::IdentifierExpression>(mod.RegisterSymbol("var"), "var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod.RegisterSymbol("determinant"),
                                        "determinant"),
      params);

  mod.AddGlobalVariable(var);

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
