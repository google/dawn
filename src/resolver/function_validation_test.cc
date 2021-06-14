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
           Return(2),
       },
       ast::DecorationList{});

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           Return(2),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(12:34 error v-0016: duplicate function named 'func'
note: first function declared here)");
}

TEST_F(ResolverFunctionValidationTest,
       VoidFunctionEndWithoutReturnStatement_Pass) {
  // fn func { var a:i32 = 2; }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           Decl(var),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionNameSameAsGlobalVariableName_Fail) {
  // var foo:f32 = 3.14;
  // fn foo() -> void {}

  auto* global_var = Var(Source{Source::Location{56, 78}}, "foo", ty.f32(),
                         ast::StorageClass::kPrivate, Expr(3.14f));
  AST().AddGlobalVariable(global_var);

  Func(Source{Source::Location{12, 34}}, "foo", ast::VariableList{}, ty.void_(),
       ast::StatementList{}, ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-2000: duplicate declaration 'foo'\n56:78 note: "
            "'foo' first declared here:");
}

TEST_F(ResolverFunctionValidationTest,
       GlobalVariableNameSameAFunctionName_Fail) {
  // fn foo() -> void {}
  // var<private> foo:f32 = 3.14;

  Func(Source{Source::Location{12, 34}}, "foo", ast::VariableList{}, ty.void_(),
       ast::StatementList{}, ast::DecorationList{});
  auto* global_var =
      Var("foo", ty.f32(), ast::StorageClass::kPrivate, Expr(3.14f));
  AST().AddGlobalVariable(global_var);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "error v-2000: duplicate declaration 'foo'\n12:34 note: 'foo' "
            "first declared here:");
}

TEST_F(ResolverFunctionValidationTest, FunctionUsingSameVariableName_Pass) {
  // fn func() -> i32 {
  //   var func:i32 = 0;
  //   return func;
  // }

  auto* var = Var("func", ty.i32(), ast::StorageClass::kNone, Expr(0));
  Func("func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           Decl(var),
           Return(Source{Source::Location{12, 34}}, Expr("func")),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionNameSameAsFunctionScopeVariableName_Pass) {
  // fn a() -> void { var b:i32 = 0; }
  // fn b() -> i32 { return 2; }

  auto* var = Var("b", ty.i32(), ast::StorageClass::kNone, Expr(0));
  Func("a", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(var),
       },
       ast::DecorationList{});

  Func(Source{Source::Location{12, 34}}, "b", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           Return(2),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> int { var a:i32 = 2; }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           Decl(var),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0002: non-void function must end with a return statement");
}

TEST_F(ResolverFunctionValidationTest,
       VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
  // fn func {}

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(), ast::StatementList{});

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
  // fn func { return; }

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Return(),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func { return 2; }
  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Return(Source{Source::Location{12, 34}}, Expr(2)),
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
           Return(Source{Source::Location{12, 34}}, Expr(2.f)),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           Return(Source{Source::Location{12, 34}}, Expr(2)),
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
  auto* myf32 = Alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, ty.Of(myf32),
       ast::StatementList{
           Return(Source{Source::Location{12, 34}}, Expr(2.f)),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest,
       FunctionTypeMustMatchReturnStatementTypeF32Alias_fail) {
  // type myf32 = f32;
  // fn func -> myf32 { return 2; }
  auto* myf32 = Alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, ty.Of(myf32),
       ast::StatementList{
           Return(Source{Source::Location{12, 34}}, Expr(2u)),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000y: return statement type must match its function "
            "return type, returned 'u32', expected 'myf32'");
}

TEST_F(ResolverFunctionValidationTest, PipelineStage_MustBeUnique_Fail) {
  // [[stage(fragment)]]
  // [[stage(vertex)]]
  // fn main() { return; }
  Func(Source{Source::Location{12, 34}}, "main", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{
           Stage(Source{{12, 34}}, ast::PipelineStage::kVertex),
           Stage(Source{{56, 78}}, ast::PipelineStage::kFragment),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(56:78 error: duplicate stage decoration
12:34 note: first decoration declared here)");
}

TEST_F(ResolverFunctionValidationTest, NoPipelineEntryPoints) {
  Func("vtx_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionVarInitWithParam) {
  // fn foo(bar : f32){
  //   var baz : f32 = bar;
  // }

  auto* bar = Param("bar", ty.f32());
  auto* baz = Var("baz", ty.f32(), ast::StorageClass::kNone, Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionConstInitWithParam) {
  // fn foo(bar : f32){
  //   let baz : f32 = bar;
  // }

  auto* bar = Param("bar", ty.f32());
  auto* baz = Const("baz", ty.f32(), Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_BadType) {
  // [[stage(compute), workgroup_size(64.0)]
  // fn main() {}

  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(create<ast::ScalarConstructorExpression>(
            Source{Source::Location{12, 34}}, Literal(64.f)))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: workgroup_size parameter must be a literal i32 or an "
            "i32 module-scope constant");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Negative) {
  // [[stage(compute), workgroup_size(-2)]
  // fn main() {}

  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(create<ast::ScalarConstructorExpression>(
            Source{Source::Location{12, 34}}, Literal(-2)))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: workgroup_size parameter must be a positive i32 value");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Zero) {
  // [[stage(compute), workgroup_size(0)]
  // fn main() {}

  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(create<ast::ScalarConstructorExpression>(
            Source{Source::Location{12, 34}}, Literal(0)))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: workgroup_size parameter must be a positive i32 value");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_BadType) {
  // let x = 64.0;
  // [[stage(compute), workgroup_size(x)]
  // fn main() {}
  GlobalConst("x", ty.f32(), Expr(64.f));
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(Expr(Source{Source::Location{12, 34}}, "x"))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: workgroup_size parameter must be a literal i32 or an "
            "i32 module-scope constant");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Negative) {
  // let x = -2;
  // [[stage(compute), workgroup_size(x)]
  // fn main() {}
  GlobalConst("x", ty.i32(), Expr(-2));
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(Expr(Source{Source::Location{12, 34}}, "x"))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: workgroup_size parameter must be a positive i32 value");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Zero) {
  // let x = 0;
  // [[stage(compute), workgroup_size(x)]
  // fn main() {}
  GlobalConst("x", ty.i32(), Expr(0));
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(Expr(Source{Source::Location{12, 34}}, "x"))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: workgroup_size parameter must be a positive i32 value");
}

TEST_F(ResolverFunctionValidationTest,
       WorkgroupSize_Const_NestedZeroValueConstructor) {
  // let x = i32(i32(i32()));
  // [[stage(compute), workgroup_size(x)]
  // fn main() {}
  GlobalConst("x", ty.i32(),
              Construct(ty.i32(), Construct(ty.i32(), Construct(ty.i32()))));
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(Expr(Source{Source::Location{12, 34}}, "x"))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: workgroup_size parameter must be a positive i32 value");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_NonConst) {
  // var<private> x = 0;
  // [[stage(compute), workgroup_size(x)]
  // fn main() {}
  Global("x", ty.i32(), ast::StorageClass::kPrivate, Expr(64));
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute),
        WorkgroupSize(Expr(Source{Source::Location{12, 34}}, "x"))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: workgroup_size parameter must be a literal i32 or an "
            "i32 module-scope constant");
}

}  // namespace
}  // namespace tint
