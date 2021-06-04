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

struct ResolverPtrRefTest : public resolver::TestHelper,
                            public testing::Test {};

TEST_F(ResolverPtrRefTest, AddressOf) {
  // var v : i32;
  // &v

  auto* v = Var("v", ty.i32(), ast::StorageClass::kNone);
  auto* expr = AddressOf(v);

  WrapInFunction(v, expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(expr)->Is<sem::Pointer>());
  EXPECT_TRUE(TypeOf(expr)->As<sem::Pointer>()->StoreType()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(expr)->As<sem::Pointer>()->StorageClass(),
            ast::StorageClass::kFunction);
}

TEST_F(ResolverPtrRefTest, AddressOfThenDeref) {
  // var v : i32;
  // *(&v)

  auto* v = Var("v", ty.i32(), ast::StorageClass::kNone);
  auto* expr = Deref(AddressOf(v));

  WrapInFunction(v, expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(expr)->Is<sem::Reference>());
  EXPECT_TRUE(TypeOf(expr)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
}

TEST_F(ResolverPtrRefTest, DefaultStorageClass) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

  auto* buf = Structure("S", {Member("m", ty.i32())},
                        {create<ast::StructBlockDecoration>()});
  auto* function = Var("f", ty.i32());
  auto* private_ = Global("p", ty.i32(), ast::StorageClass::kPrivate);
  auto* workgroup = Global("w", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* uniform = Global("ub", buf, ast::StorageClass::kUniform,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(0),
                         });
  auto* storage = Global("sb", buf, ast::StorageClass::kStorage,
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

TEST_F(ResolverPtrRefTest, ExplicitStorageClass) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

  auto* buf = Structure("S", {Member("m", ty.i32())},
                        {create<ast::StructBlockDecoration>()});
  auto* storage =
      Global("sb", buf, ast::StorageClass::kStorage, ast::Access::kReadWrite,
             ast::DecorationList{
                 create<ast::BindingDecoration>(1),
                 create<ast::GroupDecoration>(0),
             });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());

  EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(),
            ast::Access::kReadWrite);
}

TEST_F(ResolverPtrRefTest, InheritsAccessFromOriginatingVariable) {
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
  auto* buf = Structure("S", {Member("inner", inner)},
                        {create<ast::StructBlockDecoration>()});
  auto* storage =
      Global("s", buf, ast::StorageClass::kStorage, ast::Access::kReadWrite,
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
