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

#include "src/validator_impl.h"

#include "gtest/gtest.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/call_statement.h"
#include "src/ast/entry_point.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type_determiner.h"
#include "src/validator_test_helper.h"

namespace tint {
namespace {

class ValidateFunctionTest : public ValidatorTestHelper,
                             public testing::Test {};

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> void { var a:i32 = 2; }

  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::VariableList params;
  ast::type::VoidType void_type;
  auto func = std::make_unique<ast::Function>(Source{12, 34}, "func",
                                              std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0002: function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
  // fn func -> void {}
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>(Source{12, 34}, "func",
                                              std::move(params), &void_type);
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0002: function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_Pass) {
  // fn func -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));
  AddFakeEntryPoint();

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func -> void { return 2; }
  ast::type::VoidType void_type;
  ast::type::I32Type i32;
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(Source{12, 34},
                                                      std::move(return_expr)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v()->error(),
            "12:34: v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__void'");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("func", std::move(params), &f32);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(Source{12, 34},
                                                      std::move(return_expr)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v()->error(),
            "12:34: v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__f32'");
}

TEST_F(ValidateFunctionTest, FunctionNamesMustBeUnique_fail) {
  // fn func -> i32 { return 2; }
  // fn func -> i32 { return 2; }
  ast::type::VoidType void_type;
  ast::type::I32Type i32;

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("func", std::move(params), &i32);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(std::move(return_expr)));
  func->set_body(std::move(body));

  ast::VariableList params_copy;
  auto func_copy = std::make_unique<ast::Function>(
      Source{12, 34}, "func", std::move(params_copy), &i32);
  auto body_copy = std::make_unique<ast::BlockStatement>();
  auto return_expr_copy = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body_copy->append(
      std::make_unique<ast::ReturnStatement>(std::move(return_expr_copy)));
  func_copy->set_body(std::move(body_copy));

  mod()->AddFunction(std::move(func));
  mod()->AddFunction(std::move(func_copy));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0016: function names must be unique 'func'");
}

TEST_F(ValidateFunctionTest, RecursionIsNotAllowed_Fail) {
  // fn func() -> void {func(); return; }
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::ExpressionList call_params;
  auto call_expr = std::make_unique<ast::CallExpression>(
      Source{12, 34}, std::make_unique<ast::IdentifierExpression>("func"),
      std::move(call_params));
  ast::VariableList params0;
  auto func0 =
      std::make_unique<ast::Function>("func", std::move(params0), &f32);
  auto body0 = std::make_unique<ast::BlockStatement>();
  body0->append(std::make_unique<ast::CallStatement>(std::move(call_expr)));
  body0->append(std::make_unique<ast::ReturnStatement>());
  func0->set_body(std::move(body0));
  mod()->AddFunction(std::move(func0));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod())) << v()->error();
  EXPECT_EQ(v()->error(), "12:34: v-0004: recursion is not allowed: 'func'");
}

TEST_F(ValidateFunctionTest, RecursionIsNotAllowedExpr_Fail) {
  // fn func() -> i32 {var a: i32 = func(); return 2; }
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  ast::ExpressionList call_params;
  auto call_expr = std::make_unique<ast::CallExpression>(
      Source{12, 34}, std::make_unique<ast::IdentifierExpression>("func"),
      std::move(call_params));
  var->set_constructor(std::move(call_expr));
  ast::VariableList params0;
  auto func0 =
      std::make_unique<ast::Function>("func", std::move(params0), &i32);
  auto body0 = std::make_unique<ast::BlockStatement>();
  body0->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body0->append(std::make_unique<ast::ReturnStatement>(std::move(return_expr)));
  func0->set_body(std::move(body0));
  mod()->AddFunction(std::move(func0));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod())) << v()->error();
  EXPECT_EQ(v()->error(), "12:34: v-0004: recursion is not allowed: 'func'");
}

TEST_F(ValidateFunctionTest, EntryPointFunctionMissing_Fail) {
  // entry_point vertex as "main" = vtx_main
  // fn frag_main() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_main", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kVertex, "main", "frag_main");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0019: Function used in entry point does not exist: "
            "'frag_main'");
}

TEST_F(ValidateFunctionTest, EntryPointFunctionExist_Pass) {
  // entry_point vertex as "main" = vtx_main
  // fn vtx_main() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_main", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kVertex, "main", "vtx_main");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidateFunctionTest, EntryPointFunctionNotVoid_Fail) {
  // entry_point vertex as "main" = vtx_main
  // fn vtx_main() -> i32 { return 0; }
  ast::type::I32Type i32;
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("vtx_main", std::move(params), &i32);
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 0));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(std::move(return_expr)));
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kVertex, "main", "vtx_main");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0024: Entry point function must return void: 'vtx_main'");
}

TEST_F(ValidateFunctionTest, EntryPointFunctionWithParams_Fail) {
  // entry_point vertex as "func" = vtx_func
  // fn vtx_func(a : i32) -> void { return; }
  ast::type::I32Type i32;
  ast::type::VoidType void_type;
  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32));
  auto func = std::make_unique<ast::Function>("vtx_func", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kVertex, "func", "vtx_func");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0023: Entry point function must accept no parameters: "
            "'vtx_func'");
}

TEST_F(ValidateFunctionTest, EntryPointFunctionPairMustBeUniqueDuplicate_Fail) {
  // entry_point vertex  = vtx_main
  // entry_point vertex  = vtx_main
  // fn vtx_main() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_main", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kVertex, "", "vtx_main");
  auto entry_point_copy = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kVertex, "", "vtx_main");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  mod()->AddEntryPoint(std::move(entry_point_copy));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0020: The pair of <entry point name, pipeline stage> "
            "must be unique");
}

TEST_F(ValidateFunctionTest, EntryPointFunctionPairMustBeUniqueTowVertex_Fail) {
  // entry_point vertex as "main" = vtx_func1
  // entry_point vertex as "main" = vtx_func0
  // fn vtx_func1() -> void { return; }
  // fn vtx_func0() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_func0", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  ast::VariableList params1;
  auto func1 = std::make_unique<ast::Function>("vtx_func1", std::move(params1),
                                               &void_type);
  auto body1 = std::make_unique<ast::BlockStatement>();
  body1->append(std::make_unique<ast::ReturnStatement>());
  func1->set_body(std::move(body1));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kVertex, "main", "vtx_func0");
  auto entry_point1 = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kVertex, "main", "vtx_func1");

  mod()->AddFunction(std::move(func));
  mod()->AddFunction(std::move(func1));
  mod()->AddEntryPoint(std::move(entry_point));
  mod()->AddEntryPoint(std::move(entry_point1));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0020: The pair of <entry point name, pipeline stage> "
            "must be unique");
}

TEST_F(ValidateFunctionTest,
       EntryPointFunctionPairMustBeUniqueSameFuncDiffStage_Pass) {
  // entry_point vertex as "main" = vtx_func
  // entry_point fragment as "main" = vtx_func
  // fn vtx_func() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_func", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kVertex, "main", "vtx_func");
  auto entry_point1 = std::make_unique<ast::EntryPoint>(
      Source{12, 34}, ast::PipelineStage::kFragment, "main", "vtx_func");

  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));
  mod()->AddEntryPoint(std::move(entry_point1));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidateFunctionTest,
       AtLeastOneOfVertexFragmentComputeShaderMustBePeresent_Pass) {
  // entry_point vertex as "main" = vtx_func
  // fn vtx_func() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_func", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  auto entry_point = std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kVertex, "main", "vtx_func");
  mod()->AddFunction(std::move(func));
  mod()->AddEntryPoint(std::move(entry_point));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidateFunctionTest,
       AtLeastOneOfVertexFragmentComputeShaderMustBePeresent_Fail) {
  // fn vtx_func() -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("vtx_func", std::move(params),
                                              &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "0:0: v-0003: At least one of vertex, fragment or compute shader "
            "must be present");
}

}  // namespace
}  // namespace tint
