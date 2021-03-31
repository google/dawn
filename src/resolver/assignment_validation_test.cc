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

#include "gmock/gmock.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/type/access_control_type.h"
#include "src/type/storage_texture_type.h"

namespace tint {
namespace resolver {
namespace {

using ResolverAssignmentValidationTest = ResolverTest;

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypes) {
  // {
  //  var a : i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(var, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignThroughPointerWrongeStoreType_Fail) {
  // var a : f32;
  // const b : ptr<function,f32> = a;
  // b = 2;
  const auto priv = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.f32(), priv);
  auto* var_b = Const("b", ty.pointer<float>(priv), Expr("a"), {});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(var_a, var_b, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'i32' to a variable of type 'f32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a : i32 = 2;
  //  a = 2
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });
  WrapInFunction(var, body);

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverAssignmentValidationTest,
       AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a : i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });
  WrapInFunction(var, block);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignIncompatibleTypesInNestedBlockStatement_Fail) {
  // {
  //  {
  //   var a : i32 = 2;
  //   a = 2.3;
  //  }
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* inner_block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });

  auto* outer_block = create<ast::BlockStatement>(ast::StatementList{
      inner_block,
  });

  WrapInFunction(var, outer_block);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest, AssignToScalar_Fail) {
  // var my_var : i32 = 2;
  // 1 = my_var;

  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr(1);
  auto* rhs = Expr("my_var");

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var), assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000x: invalid assignment: left-hand-side does not "
            "reference storage: i32");
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypes_Pass) {
  // var a :i32 = 2;
  // a = 2
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesThroughAlias_Pass) {
  // alias myint = i32;
  // var a :myint = 2;
  // a = 2
  auto* myint = ty.alias("myint", ty.i32());
  auto* var = Var("a", myint, ast::StorageClass::kNone, Expr(2));

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesInferRHSLoad_Pass) {
  // var a : i32 = 2;
  // var b : i32 = 3;
  // a = b;
  auto* var_a = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* var_b = Var("b", ty.i32(), ast::StorageClass::kNone, Expr(3));

  auto* lhs = Expr("a");
  auto* rhs = Expr("b");

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var_a), Decl(var_b), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignThroughPointer_Pass) {
  // var a :i32;
  // const b : ptr<function,i32> = a;
  // b = 2;
  const auto func = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.i32(), func, Expr(2), {});
  auto* var_b = Const("b", ty.pointer<int>(func), Expr("a"), {});

  auto* lhs = Expr("b");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var_a), Decl(var_b), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignToConstant_Fail) {
  // {
  //  const a :i32 = 2;
  //  a = 2
  // }
  auto* var = Const("a", ty.i32(), Expr(2));

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  WrapInFunction(body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0021: cannot re-assign a constant: 'a'");
}

TEST_F(ResolverAssignmentValidationTest, AssignFromPointer_Fail) {
  // var a : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // var b : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // a = b;

  auto* tex_type = create<type::StorageTexture>(
      type::TextureDimension::k1d, type::ImageFormat::kRgba8Unorm,
      type::StorageTexture::SubtypeFor(type::ImageFormat::kRgba8Unorm,
                                       Types()));
  auto* tex_ac =
      create<type::AccessControl>(ast::AccessControl::kReadOnly, tex_type);

  auto* var_a = Var("a", tex_ac, ast::StorageClass::kFunction);
  auto* var_b = Var("b", tex_ac, ast::StorageClass::kFunction);

  auto* lhs = Expr("a");
  auto* rhs = Expr("b");

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var_a), Decl(var_b), assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000x: invalid assignment: right-hand-side is not "
            "storable: ptr<function, [[access(read)]] "
            "texture_storage_1d<rgba8unorm>>");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
