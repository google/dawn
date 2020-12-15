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

#include "gtest/gtest.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/call_statement.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type_determiner.h"
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidateFunctionTest : public ValidatorTestHelper,
                             public testing::Test {};

TEST_F(ValidateFunctionTest, VoidFunctionEndWithoutReturnStatement_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { var a:i32 = 2; }
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32, Expr(2),
                  ast::VariableDecorationList{});

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("func"), "func",
      params, ty.void_, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod));
}

TEST_F(ValidateFunctionTest,
       VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
  // [[stage(vertex)]]
  // fn func -> void {}
  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("func"), "func",
      params, ty.void_, create<ast::BlockStatement>(ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod));
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> int { var a:i32 = 2; }

  auto* var = Var("a", ast::StorageClass::kNone, ty.i32, Expr(2),
                  ast::VariableDecorationList{});

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("func"), "func",
      params, ty.i32, body, ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "12:34 v-0002: non-void function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
  // fn func -> int {}
  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("func"), "func",
      params, ty.i32, create<ast::BlockStatement>(ast::StatementList{}),
      ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "12:34 v-0002: non-void function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { return; }
  ast::VariableList params;

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* func = create<ast::Function>(
      mod->RegisterSymbol("func"), "func", params, ty.void_, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });
  mod->AddFunction(func);

  EXPECT_TRUE(td()->DetermineFunctions(mod->functions())) << td()->error();
  EXPECT_TRUE(v()->ValidateFunctions(mod->functions())) << v()->error();
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func -> void { return 2; }
  ast::VariableList params;
  auto* return_expr = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                   return_expr),
  });

  auto* func =
      create<ast::Function>(mod->RegisterSymbol("func"), "func", params,
                            ty.void_, body, ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v()->error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__void'");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  ast::VariableList params;
  auto* return_expr = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                   return_expr),
  });

  auto* func =
      create<ast::Function>(mod->RegisterSymbol("func"), "func", params, ty.f32,
                            body, ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v()->error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__f32'");
}

TEST_F(ValidateFunctionTest, FunctionNamesMustBeUnique_fail) {
  // fn func -> i32 { return 2; }
  // fn func -> i32 { return 2; }

  ast::VariableList params;
  auto* return_expr = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(return_expr),
  });
  auto* func =
      create<ast::Function>(mod->RegisterSymbol("func"), "func", params, ty.i32,
                            body, ast::FunctionDecorationList{});

  ast::VariableList params_copy;
  auto* return_expr_copy = Expr(2);
  auto* body_copy = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(return_expr_copy),
  });

  auto* func_copy = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("func"), "func",
      params_copy, ty.i32, body_copy, ast::FunctionDecorationList{});

  mod->AddFunction(func);
  mod->AddFunction(func_copy);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(), "12:34 v-0016: function names must be unique 'func'");
}

TEST_F(ValidateFunctionTest, RecursionIsNotAllowed_Fail) {
  // fn func() -> void {func(); return; }
  ast::ExpressionList call_params;
  auto* call_expr = create<ast::CallExpression>(
      Source{Source::Location{12, 34}}, Expr("func"), call_params);
  ast::VariableList params0;
  auto* body0 = create<ast::BlockStatement>(ast::StatementList{
      create<ast::CallStatement>(call_expr),
      create<ast::ReturnStatement>(),
  });
  auto* func0 =
      create<ast::Function>(mod->RegisterSymbol("func"), "func", params0,
                            ty.f32, body0, ast::FunctionDecorationList{});
  mod->AddFunction(func0);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod)) << v()->error();
  EXPECT_EQ(v()->error(), "12:34 v-0004: recursion is not allowed: 'func'");
}

TEST_F(ValidateFunctionTest, RecursionIsNotAllowedExpr_Fail) {
  // fn func() -> i32 {var a: i32 = func(); return 2; }
  ast::ExpressionList call_params;
  auto* call_expr = create<ast::CallExpression>(
      Source{Source::Location{12, 34}}, Expr("func"), call_params);
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32, call_expr,
                  ast::VariableDecorationList{});

  ast::VariableList params0;
  auto* return_expr = Expr(2);

  auto* body0 = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::ReturnStatement>(return_expr),
  });

  auto* func0 =
      create<ast::Function>(mod->RegisterSymbol("func"), "func", params0,
                            ty.i32, body0, ast::FunctionDecorationList{});
  mod->AddFunction(func0);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod)) << v()->error();
  EXPECT_EQ(v()->error(), "12:34 v-0004: recursion is not allowed: 'func'");
}

TEST_F(ValidateFunctionTest, Function_WithPipelineStage_NotVoid_Fail) {
  // [[stage(vertex)]]
  // fn vtx_main() -> i32 { return 0; }
  ast::VariableList params;
  auto* return_expr = Expr(0);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(return_expr),
  });
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("vtx_main"),
      "vtx_main", params, ty.i32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  mod->AddFunction(func);
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "12:34 v-0024: Entry point function must return void: 'vtx_main'");
}

TEST_F(ValidateFunctionTest, Function_WithPipelineStage_WithParams_Fail) {
  // [[stage(vertex)]]
  // fn vtx_func(a : i32) -> void { return; }
  ast::VariableList params;
  params.push_back(Var("a", ast::StorageClass::kNone, ty.i32, nullptr,
                       ast::VariableDecorationList{}));
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("vtx_func"),
      "vtx_func", params, ty.void_, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  mod->AddFunction(func);
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "12:34 v-0023: Entry point function must accept no parameters: "
            "'vtx_func'");
}

TEST_F(ValidateFunctionTest, PipelineStage_MustBeUnique_Fail) {
  // [[stage(fragment)]]
  // [[stage(vertex)]]
  // fn main() -> void { return; }
  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* func = create<ast::Function>(
      Source{Source::Location{12, 34}}, mod->RegisterSymbol("main"), "main",
      params, ty.void_, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  mod->AddFunction(func);
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(
      v()->error(),
      "12:34 v-0020: only one stage decoration permitted per entry point");
}

TEST_F(ValidateFunctionTest, OnePipelineStageFunctionMustBePresent_Pass) {
  // [[stage(vertex)]]
  // fn vtx_func() -> void { return; }
  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* func = create<ast::Function>(
      mod->RegisterSymbol("vtx_func"), "vtx_func", params, ty.void_, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod)) << v()->error();
}

TEST_F(ValidateFunctionTest, OnePipelineStageFunctionMustBePresent_Fail) {
  // fn vtx_func() -> void { return; }
  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* func =
      create<ast::Function>(mod->RegisterSymbol("vtx_func"), "vtx_func", params,
                            ty.void_, body, ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "v-0003: At least one of vertex, fragment or compute shader must "
            "be present");
}

}  // namespace
}  // namespace tint
