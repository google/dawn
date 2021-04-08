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
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace {

class ResolverTypeValidationTest : public resolver::TestHelper,
                                   public testing::Test {};

TEST_F(ResolverTypeValidationTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, nullptr);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  WrapInFunction(body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
}

TEST_F(ResolverTypeValidationTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> global_var: f32;
  Global(Source{{12, 34}}, "global_var", ty.f32(), ast::StorageClass::kInput);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableNoStorageClass_Fail) {
  // var global_var: f32;
  Global(Source{{12, 34}}, "global_var", ty.f32(), ast::StorageClass::kNone);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0022: global variables must have a storage class");
}

TEST_F(ResolverTypeValidationTest, GlobalConstantWithStorageClass_Fail) {
  // const<in> global_var: f32;
  AST().AddGlobalVariable(
      create<ast::Variable>(Source{{12, 34}}, Symbols().Register("global_var"),
                            ast::StorageClass::kInput, ty.f32(), true, nullptr,
                            ast::DecorationList{}));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-global01: global constants shouldn't have a storage "
            "class");
}

TEST_F(ResolverTypeValidationTest, GlobalConstNoStorageClass_Pass) {
  // const global_var: f32;
  GlobalConst(Source{{12, 34}}, "global_var", ty.f32());

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;

  Global("global_var0", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  Global(Source{{12, 34}}, "global_var1", ty.f32(), ast::StorageClass::kPrivate,
         Expr(0));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableNotUnique_Fail) {
  // var global_var : f32 = 0.1;
  // var global_var : i32 = 0;
  Global("global_var", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  Global(Source{{12, 34}}, "global_var", ty.i32(), ast::StorageClass::kPrivate,
         Expr(0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0011: redeclared global identifier 'global_var'");
}

TEST_F(ResolverTypeValidationTest,
       GlobalVariableFunctionVariableNotUnique_Pass) {
  // fn my_func() {
  //   var a: f32 = 2.0;
  // }
  // var a: f32 = 2.1;

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(), {Decl(var)},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest,
       GlobalVariableFunctionVariableNotUnique_Fail) {
  // var a: f32 = 2.1;
  // fn my_func() {
  //   var a: f32 = 2.0;
  //   return 0;
  // }

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{{12, 34}}, var),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(), "12:34 error v-0013: redeclared identifier 'a'");
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifier_Fail) {
  // fn my_func()() {
  //  var a :i32 = 2;
  //  var a :f21 = 2.0;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(0.1f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::VariableDeclStatement>(Source{{12, 34}}, var_a_float),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'a'");
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
      create<ast::VariableDeclStatement>(Source{{12, 34}}, var_a_float),
  });

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeValidationTest,
       DISABLED_RedeclaredIdentifierInnerScope_False) {
  // TODO(sarahM0): remove DISABLED after implementing ValidateIfStatement
  // and it should just work
  // {
  // var a : f32 = 3.14;
  // if (true) { var a : f32 = 2.0; }
  // }
  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{{12, 34}}, var),
  });

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_a_float),
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
  });

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'a'");
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
  // {
  //  { var a : f32; }
  //  var a : f32;
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{{12, 34}}, var_inner),
  });

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      inner,
      create<ast::VariableDeclStatement>(var_outer),
  });

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScopeBlock_Fail) {
  // {
  //  var a : f32;
  //  { var a : f32; }
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{{12, 34}}, var_inner),
  });

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_outer),
      inner,
  });

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'a'");
}

TEST_F(ResolverTypeValidationTest,
       RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  auto* var0 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* var1 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(1.0f));

  Func("func0", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{{12, 34}}, var0),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  Func("func1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{{13, 34}}, var1),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInFunction_Fail) {
  /// [[stage(vertex)]]
  // fn func() { var a : array<i32>; }

  auto* var =
      Var(Source{{12, 34}}, "a", ty.array<i32>(), ast::StorageClass::kNone);

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLast_Pass) {
  // [[Block]]
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32()),
                                                Member("rt", ty.array<f32>())},
                          decos);

  auto* struct_type = ty.struct_("Foo", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLastNoBlock_Fail) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::DecorationList decos;
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("vf", ty.f32()),
                            Member(Source{{12, 34}}, "rt", ty.array<f32>())},
      decos);

  auto* struct_type = ty.struct_("Foo", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-0015: a struct containing a runtime-sized array "
            "requires the [[block]] attribute: 'Foo'");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // struct Foo {
  //   rt: array<f32>;
  //   vf: f32;
  // };

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());

  auto* rt = Member(Source{{12, 34}}, "rt", ty.array<f32>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{rt, Member("vf", ty.f32())}, decos);

  auto* struct_type = ty.struct_("Foo", st);

  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-0015: runtime arrays may only appear as the last "
            "member of a struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsParameter_Fail) {
  // fn func(a : array<u32>) {}
  // [[stage(vertex)]] fn main() {}

  auto* param =
      Var(Source{{12, 34}}, "a", ty.array<i32>(), ast::StorageClass::kNone);

  Func("func", ast::VariableList{param}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member(Source{{12, 34}}, "b", alias),
                            Member("a", ty.u32())},
      decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsLast_Pass) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.u32()), Member("b", alias)}, decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint
