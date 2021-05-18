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

#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

struct ResolverVarLetValidationTest : public resolver::TestHelper,
                                      public testing::Test {};

TEST_F(ResolverVarLetValidationTest, LetNoInitializer) {
  // let a : i32;
  WrapInFunction(Const(Source{{12, 34}}, "a", ty.i32(), nullptr));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: let declarations must have initializers");
}

TEST_F(ResolverVarLetValidationTest, GlobalLetNoInitializer) {
  // let a : i32;
  GlobalConst(Source{{12, 34}}, "a", ty.i32(), nullptr);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: let declarations must have initializers");
}

TEST_F(ResolverVarLetValidationTest, VarConstructorNotStorable) {
  // var i : i32;
  // var p : pointer<function, i32> = &v;
  auto* i = Var("i", ty.i32(), ast::StorageClass::kNone);
  auto* p = Var("a", ty.i32(), ast::StorageClass::kNone,
                AddressOf(Source{{12, 34}}, "i"));
  WrapInFunction(i, p);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: 'ptr<function, i32>' is not storable for assignment");
}

TEST_F(ResolverVarLetValidationTest, LetConstructorWrongType) {
  // var v : i32 = 2u
  WrapInFunction(Const(Source{{3, 3}}, "v", ty.i32(), Expr(2u)));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: cannot initialize let of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, VarConstructorWrongType) {
  // var v : i32 = 2u
  WrapInFunction(
      Var(Source{{3, 3}}, "v", ty.i32(), ast::StorageClass::kNone, Expr(2u)));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: cannot initialize var of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, LetConstructorWrongTypeViaAlias) {
  auto* a = ty.alias("I32", ty.i32());
  AST().AddConstructedType(a);
  WrapInFunction(Const(Source{{3, 3}}, "v", a, Expr(2u)));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: cannot initialize let of type 'I32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, VarConstructorWrongTypeViaAlias) {
  auto* a = ty.alias("I32", ty.i32());
  AST().AddConstructedType(a);
  WrapInFunction(
      Var(Source{{3, 3}}, "v", a, ast::StorageClass::kNone, Expr(2u)));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: cannot initialize var of type 'I32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, LetOfPtrConstructedWithRef) {
  // var a : f32;
  // let b : ptr<function,f32> = a;
  const auto priv = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.f32(), priv);
  auto* var_b =
      Const(Source{{12, 34}}, "b", ty.pointer<float>(priv), Expr("a"), {});
  WrapInFunction(var_a, var_b);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: cannot initialize let of type 'ptr<function, f32>' with value of type 'f32')");
}

TEST_F(ResolverVarLetValidationTest, LocalVarRedeclared) {
  // var v : f32;
  // var v : i32;
  auto* v1 = Var("v", ty.f32(), ast::StorageClass::kNone);
  auto* v2 = Var(Source{{12, 34}}, "v", ty.i32(), ast::StorageClass::kNone);
  WrapInFunction(v1, v2);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'v'");
}

TEST_F(ResolverVarLetValidationTest, LocalLetRedeclared) {
  // let l : f32 = 1.;
  // let l : i32 = 0;
  auto* l1 = Const("l", ty.f32(), Expr(1.f));
  auto* l2 = Const(Source{{12, 34}}, "l", ty.i32(), Expr(0));
  WrapInFunction(l1, l2);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'l'");
}

TEST_F(ResolverVarLetValidationTest, GlobalVarRedeclared) {
  // var v : f32;
  // var v : i32;
  Global("v", ty.f32(), ast::StorageClass::kPrivate);
  Global(Source{{12, 34}}, "v", ty.i32(), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0011: redeclared global identifier 'v'");
}

TEST_F(ResolverVarLetValidationTest, GlobalLetRedeclared) {
  // let l : f32 = 0.1;
  // let l : i32 = 0;
  GlobalConst("l", ty.f32(), Expr(0.1f));
  GlobalConst(Source{{12, 34}}, "l", ty.i32(), Expr(0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0011: redeclared global identifier 'l'");
}

TEST_F(ResolverVarLetValidationTest, GlobalVarRedeclaredAsLocal) {
  // var v : f32 = 2.1;
  // fn my_func() {
  //   var v : f32 = 2.0;
  //   return 0;
  // }

  Global("v", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  WrapInFunction(Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone,
                     Expr(2.0f)));

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(), "12:34 error v-0013: redeclared identifier 'v'");
}

TEST_F(ResolverVarLetValidationTest, VarRedeclaredInInnerBlock) {
  // {
  //  var v : f32;
  //  { var v : f32; }
  // }
  auto* var_outer = Var("v", ty.f32(), ast::StorageClass::kNone);
  auto* var_inner =
      Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone);
  auto* inner = Block(Decl(var_inner));
  auto* outer_body = Block(Decl(var_outer), inner);

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'v'");
}

TEST_F(ResolverVarLetValidationTest, VarRedeclaredInIfBlock) {
  // {
  //   var v : f32 = 3.14;
  //   if (true) { var v : f32 = 2.0; }
  // }
  auto* var_a_float = Var("v", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* var = Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone,
                  Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = Block(Decl(var));

  auto* outer_body =
      Block(Decl(var_a_float),
            create<ast::IfStatement>(cond, body, ast::ElseStatementList{}));

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error v-0014: redeclared identifier 'v'");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
