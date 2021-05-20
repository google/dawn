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

  auto* assign = Assign(Source{{12, 34}}, "a", 2.3f);
  WrapInFunction(var, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a : i32 = 2;
  //  a = 2
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  WrapInFunction(var, Assign("a", 2));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a : i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2.3f));

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
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
  auto* inner_block = Block(Decl(var), Assign(Source{{12, 34}}, "a", 2.3f));
  auto* outer_block = Block(inner_block);
  WrapInFunction(outer_block);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignToScalar_Fail) {
  // var my_var : i32 = 2;
  // 1 = my_var;

  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  WrapInFunction(var, Assign(Expr(Source{{12, 34}}, 1), "my_var"));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: cannot assign to value of type 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypes_Pass) {
  // var a : i32 = 2;
  // a = 2
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesThroughAlias_Pass) {
  // alias myint = i32;
  // var a : myint = 2;
  // a = 2
  auto* myint = ty.alias("myint", ty.i32());
  AST().AddConstructedType(myint);
  auto* var = Var("a", myint, ast::StorageClass::kNone, Expr(2));
  WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesInferRHSLoad_Pass) {
  // var a : i32 = 2;
  // var b : i32 = 3;
  // a = b;
  auto* var_a = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* var_b = Var("b", ty.i32(), ast::StorageClass::kNone, Expr(3));
  WrapInFunction(var_a, var_b, Assign(Source{{12, 34}}, "a", "b"));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignThroughPointer_Pass) {
  // var a : i32;
  // let b : ptr<function,i32> = &a;
  // *b = 2;
  const auto func = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.i32(), func, Expr(2), {});
  auto* var_b = Const("b", ty.pointer<int>(func), AddressOf(Expr("a")), {});
  WrapInFunction(var_a, var_b, Assign(Source{{12, 34}}, Deref("b"), 2));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignToConstant_Fail) {
  // {
  //  let a : i32 = 2;
  //  a = 2
  // }
  auto* var = Const("a", ty.i32(), Expr(2));
  WrapInFunction(var, Assign(Expr(Source{{12, 34}}, "a"), 2));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: cannot assign to value of type 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonStorable_Fail) {
  // var a : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // var b : [[access(read)]] texture_storage_1d<rgba8unorm>;
  // a = b;

  auto make_type = [&] {
    auto* tex_type = ty.storage_texture(ast::TextureDimension::k1d,
                                        ast::ImageFormat::kRgba8Unorm);
    return ty.access(ast::AccessControl::kReadOnly, tex_type);
  };

  Global("a", make_type(), ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });
  Global("b", make_type(), ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(0),
         });

  WrapInFunction(Assign("a", Expr(Source{{12, 34}}, "b")));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: '[[access(read)]] texture_storage_1d<rgba8unorm>' is not storable)");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
