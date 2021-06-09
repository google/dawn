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

#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/reference_type.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

struct ResolverVarLetTest : public resolver::TestHelper,
                            public testing::Test {};

TEST_F(ResolverVarLetTest, TypeOfVar) {
  // struct S { i : i32; }
  // alias A = S;
  // fn F(){
  //   var i : i32;
  //   var u : u32;
  //   var f : f32;
  //   var b : bool;
  //   var s : S;
  //   var a : A;
  // }

  auto* S = Structure("S", {Member("i", ty.i32())});
  auto* A = Alias("A", ty.Of(S));

  auto* i = Var("i", ty.i32(), ast::StorageClass::kNone);
  auto* u = Var("u", ty.u32(), ast::StorageClass::kNone);
  auto* f = Var("f", ty.f32(), ast::StorageClass::kNone);
  auto* b = Var("b", ty.bool_(), ast::StorageClass::kNone);
  auto* s = Var("s", ty.Of(S), ast::StorageClass::kNone);
  auto* a = Var("a", ty.Of(A), ast::StorageClass::kNone);

  Func("F", {}, ty.void_(),
       {
           Decl(i),
           Decl(u),
           Decl(f),
           Decl(b),
           Decl(s),
           Decl(a),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  // `var` declarations are always of reference type
  ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

  EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
  EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
  EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
  EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
}

TEST_F(ResolverVarLetTest, TypeOfLet) {
  // struct S { i : i32; }
  // fn F(){
  //   var v : i32;
  //   let i : i32 = 1;
  //   let u : u32 = 1u;
  //   let f : f32 = 1.;
  //   let b : bool = true;
  //   let s : S = S(1);
  //   let a : A = A(1);
  //   let p : pointer<function, i32> = &V;
  // }

  auto* S = Structure("S", {Member("i", ty.i32())});
  auto* A = Alias("A", ty.Of(S));

  auto* v = Var("v", ty.i32(), ast::StorageClass::kNone);
  auto* i = Const("i", ty.i32(), Expr(1));
  auto* u = Const("u", ty.u32(), Expr(1u));
  auto* f = Const("f", ty.f32(), Expr(1.f));
  auto* b = Const("b", ty.bool_(), Expr(true));
  auto* s = Const("s", ty.Of(S), Construct(ty.Of(S), Expr(1)));
  auto* a = Const("a", ty.Of(A), Construct(ty.Of(A), Expr(1)));
  auto* p =
      Const("p", ty.pointer<i32>(ast::StorageClass::kFunction), AddressOf(v));

  Func("F", {}, ty.void_(),
       {
           Decl(v),
           Decl(i),
           Decl(u),
           Decl(f),
           Decl(b),
           Decl(s),
           Decl(a),
           Decl(p),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  // `let` declarations are always of the storage type
  EXPECT_TRUE(TypeOf(i)->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(u)->Is<sem::U32>());
  EXPECT_TRUE(TypeOf(f)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(b)->Is<sem::Bool>());
  EXPECT_TRUE(TypeOf(s)->Is<sem::Struct>());
  EXPECT_TRUE(TypeOf(a)->Is<sem::Struct>());
  ASSERT_TRUE(TypeOf(p)->Is<sem::Pointer>());
  EXPECT_TRUE(TypeOf(p)->As<sem::Pointer>()->StoreType()->Is<sem::I32>());
}

TEST_F(ResolverVarLetTest, DefaultVarStorageClass) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

  auto* buf = Structure("S", {Member("m", ty.i32())},
                        {create<ast::StructBlockDecoration>()});
  auto* function = Var("f", ty.i32());
  auto* private_ = Global("p", ty.i32(), ast::StorageClass::kPrivate);
  auto* workgroup = Global("w", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* uniform = Global("ub", ty.Of(buf), ast::StorageClass::kUniform,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(0),
                         });
  auto* storage = Global("sb", ty.Of(buf), ast::StorageClass::kStorage,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(1),
                             create<ast::GroupDecoration>(0),
                         });
  auto* handle = Global("h", ty.depth_texture(ast::TextureDimension::k2d),
                        ast::DecorationList{
                            create<ast::BindingDecoration>(2),
                            create<ast::GroupDecoration>(0),
                        });

  WrapInFunction(function);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(function)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(private_)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(workgroup)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(uniform)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(handle)->Is<sem::Reference>());

  EXPECT_EQ(TypeOf(function)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(private_)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(workgroup)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(uniform)->As<sem::Reference>()->Access(),
            ast::Access::kRead);
  EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(),
            ast::Access::kRead);
  EXPECT_EQ(TypeOf(handle)->As<sem::Reference>()->Access(), ast::Access::kRead);
}

TEST_F(ResolverVarLetTest, ExplicitVarStorageClass) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

  auto* buf = Structure("S", {Member("m", ty.i32())},
                        {create<ast::StructBlockDecoration>()});
  auto* storage = Global("sb", ty.Of(buf), ast::StorageClass::kStorage,
                         ast::Access::kReadWrite,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(1),
                             create<ast::GroupDecoration>(0),
                         });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());

  EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
}

TEST_F(ResolverVarLetTest, LetInheritsAccessFromOriginatingVariable) {
  // struct Inner {
  //    arr: array<i32, 4>;
  // }
  // [[block]] struct S {
  //    inner: Inner;
  // }
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  // fn f() {
  //   let p = &s.inner.arr[2];
  // }
  auto* inner = Structure("Inner", {Member("arr", ty.array<i32, 4>())});
  auto* buf = Structure("S", {Member("inner", ty.Of(inner))},
                        {create<ast::StructBlockDecoration>()});
  auto* storage = Global("s", ty.Of(buf), ast::StorageClass::kStorage,
                         ast::Access::kReadWrite,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(0),
                         });

  auto* expr =
      IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 4);
  auto* ptr = Const("p", nullptr, AddressOf(expr));

  WrapInFunction(ptr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(expr)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(ptr)->Is<sem::Pointer>());

  EXPECT_EQ(TypeOf(expr)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(ptr)->As<sem::Pointer>()->Access(), ast::Access::kReadWrite);
}

}  // namespace
}  // namespace resolver
}  // namespace tint
