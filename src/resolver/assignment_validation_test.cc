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
#include "src/sem/access_control_type.h"
#include "src/sem/storage_texture_type.h"

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

  auto* assign = Assign(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(var, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignThroughPointerWrongeStoreType_Fail) {
  // var a : f32;
  // let b : ptr<function,f32> = a;
  // b = 2;
  const auto priv = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.f32(), priv);
  auto* var_b = Const("b", ty.pointer<float>(priv), Expr("a"), {});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = Assign(Source{{12, 34}}, lhs, rhs);
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

  auto* body = Block(Decl(var), Assign(Source{{12, 34}}, lhs, rhs));
  WrapInFunction(body);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
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

  auto* block = Block(Decl(var), Assign(Source{{12, 34}}, lhs, rhs));
  WrapInFunction(block);

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

  auto* inner_block = Block(Decl(var), Assign(Source{{12, 34}}, lhs, rhs));

  auto* outer_block = Block(inner_block);

  WrapInFunction(outer_block);

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

  auto* assign = Assign(Source{{12, 34}}, lhs, rhs);
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

  auto* assign = Assign(Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesThroughAlias_Pass) {
  // alias myint = i32;
  // var a :myint = 2;
  // a = 2
  auto* myint = ty.alias("myint", ty.i32());
  AST().AddConstructedType(myint);
  auto* var = Var("a", myint, ast::StorageClass::kNone, Expr(2));

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = Assign(Source{Source::Location{12, 34}}, lhs, rhs);
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

  auto* assign = Assign(Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var_a), Decl(var_b), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignThroughPointer_Pass) {
  // var a :i32;
  // let b : ptr<function,i32> = a;
  // b = 2;
  const auto func = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.i32(), func, Expr(2), {});
  auto* var_b = Const("b", ty.pointer<int>(func), Expr("a"), {});

  auto* lhs = Expr("b");
  auto* rhs = Expr(2);

  auto* assign = Assign(Source{Source::Location{12, 34}}, lhs, rhs);
  WrapInFunction(Decl(var_a), Decl(var_b), assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignToConstant_Fail) {
  // {
  //  let a : i32 = 2;
  //  a = 2
  // }
  auto* var = Const("a", ty.i32(), Expr(2));

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body =
      Block(Decl(var), Assign(Source{Source::Location{12, 34}}, lhs, rhs));

  WrapInFunction(body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0021: cannot re-assign a constant: 'a'");
}

TEST_F(ResolverAssignmentValidationTest, AssignFromPointer_Fail) {
  // var a : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // var b : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // a = b;

  auto make_type = [&] {
    auto tex_type = ty.storage_texture(ast::TextureDimension::k1d,
                                       ast::ImageFormat::kRgba8Unorm);
    return ty.access(ast::AccessControl::kReadOnly, tex_type);
  };

  auto* var_a = Global("a", make_type(), ast::StorageClass::kNone);
  auto* var_b = Global("b", make_type(), ast::StorageClass::kNone);

  WrapInFunction(Assign(Source{{12, 34}}, var_a, var_b));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-000x: invalid assignment: right-hand-side is not "
            "storable: ptr<uniform_constant, [[access(read)]] "
            "texture_storage_1d<rgba8unorm>>");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
