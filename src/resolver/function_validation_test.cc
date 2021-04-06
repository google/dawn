// Copyright 2021 The Tint Authors.
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

#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace {

class ResolverFunctionValidationTest : public resolver::TestHelper,
                                       public testing::Test {};

TEST_F(ResolverFunctionValidationTest, FunctionNamesMustBeUnique_fail) {
  // fn func -> i32 { return 2; }
  // fn func -> i32 { return 2; }
  Func("func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr(2)),
       },
       ast::DecorationList{});

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr(2)),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0016: function names must be unique 'func'");
}

TEST_F(ResolverFunctionValidationTest,
       VoidFunctionEndWithoutReturnStatement_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { var a:i32 = 2; }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> int { var a:i32 = 2; }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0002: non-void function must end with a return statement");
}

TEST_F(ResolverFunctionValidationTest,
       VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
  // [[stage(vertex)]]
  // fn func -> void {}

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionEndWithoutReturnStatementEmptyBody_Fail) {
  // fn func -> int {}

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{}, ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0002: non-void function must end with a return statement");
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementType_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { return; }

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func -> void { return 2; }
  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2)),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000y: return statement type must match its function "
            "return type, returned 'i32', expected 'void'");
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32_pass) {
  // fn func -> f32 { return 2.0; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2.f)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2)),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000y: return statement type must match its function "
            "return type, returned 'i32', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32Alias_pass) {
  // type myf32 = f32;
  // fn func -> myf32 { return 2.0; }
  auto* myf32 = ty.alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, myf32,
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2.f)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32Alias_fail) {
  // type myf32 = f32;
  // fn func -> myf32 { return 2; }
  auto* myf32 = ty.alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, myf32,
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2u)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000y: return statement type must match its function "
            "return type, returned 'u32', expected 'myf32'");
}

TEST_F(ResolverFunctionValidationTest, PipelineStage_MustBeUnique_Fail) {
  // [[stage(fragment)]]
  // [[stage(vertex)]]
  // fn main() -> void { return; }
  Func(Source{Source::Location{12, 34}}, "main", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0020: only one stage decoration permitted per entry "
            "point");
}

TEST_F(ResolverFunctionValidationTest, NoPipelineEntryPoints) {
  Func("vtx_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionVarInitWithParam) {
  // fn foo(bar : f32) -> void{
  //   var baz : f32 = bar;
  // }

  auto* bar = Var("bar", ty.f32(), ast::StorageClass::kFunction);
  auto* baz = Var("baz", ty.f32(), ast::StorageClass::kFunction, Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionConstInitWithParam) {
  // fn foo(bar : f32) -> void{
  //   const baz : f32 = bar;
  // }

  auto* bar = Var("bar", ty.f32(), ast::StorageClass::kFunction);
  auto* baz = Const("baz", ty.f32(), Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint
