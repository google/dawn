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
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/storage_texture_type.h"

namespace tint {
namespace resolver {
namespace {

using ResolverAssignmentValidationTest = ResolverTest;

TEST_F(ResolverAssignmentValidationTest, ReadOnlyBuffer) {
  // [[block]] struct S { m : i32 };
  // [[group(0), binding(0)]]
  // var<storage,read> a : S;
  auto* s = Structure("S", {Member("m", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{12, 34}}, "a", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("a", "m"), 1));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: cannot store into a read-only type 'ref<storage, "
            "i32, read>'");
}

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
       AssignArraysWithDifferentSizeExpressions_Pass) {
  // let len = 4u;
  // {
  //   var a : array<f32, 4>;
  //   var b : array<f32, len>;
  //   a = b;
  // }

  GlobalConst("len", nullptr, Expr(4u));

  auto* a = Var("a", ty.array(ty.f32(), 4));
  auto* b = Var("b", ty.array(ty.f32(), "len"));

  auto* assign = Assign(Source{{12, 34}}, "a", "b");
  WrapInFunction(a, b, assign);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest,
       AssignArraysWithDifferentSizeExpressions_Fail) {
  // let len = 5u;
  // {
  //   var a : array<f32, 4>;
  //   var b : array<f32, len>;
  //   a = b;
  // }

  GlobalConst("len", nullptr, Expr(5u));

  auto* a = Var("a", ty.array(ty.f32(), 4));
  auto* b = Var("b", ty.array(ty.f32(), "len"));

  auto* assign = Assign(Source{{12, 34}}, "a", "b");
  WrapInFunction(a, b, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot assign 'array<f32, len>' to 'array<f32, 4>'");
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
  auto* myint = Alias("myint", ty.i32());
  auto* var = Var("a", ty.Of(myint), ast::StorageClass::kNone, Expr(2));
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
  auto* var_a = Var("a", ty.i32(), func, Expr(2));
  auto* var_b = Const("b", ty.pointer<int>(func), AddressOf(Expr("a")));
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
  EXPECT_EQ(r()->error(),
            "12:34 error: cannot assign to const\nnote: 'a' is declared here:");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Handle) {
  // var a : texture_storage_1d<rgba8unorm, read>;
  // var b : texture_storage_1d<rgba8unorm, read>;
  // a = b;

  auto make_type = [&] {
    return ty.storage_texture(ast::TextureDimension::k1d,
                              ast::ImageFormat::kRgba8Unorm,
                              ast::Access::kRead);
  };

  Global("a", make_type(), ast::StorageClass::kNone,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });
  Global("b", make_type(), ast::StorageClass::kNone,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(0),
         });

  WrapInFunction(Assign(Source{{56, 78}}, "a", "b"));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Atomic) {
  // [[block]] struct S { a : atomic<i32>; };
  // [[group(0), binding(0)]] var<storage, read_write> v : S;
  // v.a = v.a;

  auto* s = Structure("S", {Member("a", ty.atomic(ty.i32()))},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("v", "a"),
                        MemberAccessor("v", "a")));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_RuntimeArray) {
  // [[block]] struct S { a : array<f32>; };
  // [[group(0), binding(0)]] var<storage, read_write> v : S;
  // v.a = v.a;

  auto* s = Structure("S", {Member("a", ty.array(ty.f32()))},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("v", "a"),
                        MemberAccessor("v", "a")));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: storage type of assignment must be constructible");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
