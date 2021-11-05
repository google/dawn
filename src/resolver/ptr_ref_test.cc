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

TEST_F(ResolverPtrRefTest, DefaultPtrStorageClass) {
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

  auto* function_ptr =
      Const("f_ptr", ty.pointer(ty.i32(), ast::StorageClass::kFunction),
            AddressOf(function));
  auto* private_ptr =
      Const("p_ptr", ty.pointer(ty.i32(), ast::StorageClass::kPrivate),
            AddressOf(private_));
  auto* workgroup_ptr =
      Const("w_ptr", ty.pointer(ty.i32(), ast::StorageClass::kWorkgroup),
            AddressOf(workgroup));
  auto* uniform_ptr =
      Const("ub_ptr", ty.pointer(ty.Of(buf), ast::StorageClass::kUniform),
            AddressOf(uniform));
  auto* storage_ptr =
      Const("sb_ptr", ty.pointer(ty.Of(buf), ast::StorageClass::kStorage),
            AddressOf(storage));

  WrapInFunction(function, function_ptr, private_ptr, workgroup_ptr,
                 uniform_ptr, storage_ptr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_TRUE(TypeOf(function_ptr)->Is<sem::Pointer>())
      << "function_ptr is " << TypeOf(function_ptr)->TypeInfo().name;
  ASSERT_TRUE(TypeOf(private_ptr)->Is<sem::Pointer>())
      << "private_ptr is " << TypeOf(private_ptr)->TypeInfo().name;
  ASSERT_TRUE(TypeOf(workgroup_ptr)->Is<sem::Pointer>())
      << "workgroup_ptr is " << TypeOf(workgroup_ptr)->TypeInfo().name;
  ASSERT_TRUE(TypeOf(uniform_ptr)->Is<sem::Pointer>())
      << "uniform_ptr is " << TypeOf(uniform_ptr)->TypeInfo().name;
  ASSERT_TRUE(TypeOf(storage_ptr)->Is<sem::Pointer>())
      << "storage_ptr is " << TypeOf(storage_ptr)->TypeInfo().name;

  EXPECT_EQ(TypeOf(function_ptr)->As<sem::Pointer>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(private_ptr)->As<sem::Pointer>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(workgroup_ptr)->As<sem::Pointer>()->Access(),
            ast::Access::kReadWrite);
  EXPECT_EQ(TypeOf(uniform_ptr)->As<sem::Pointer>()->Access(),
            ast::Access::kRead);
  EXPECT_EQ(TypeOf(storage_ptr)->As<sem::Pointer>()->Access(),
            ast::Access::kRead);
}

}  // namespace
}  // namespace resolver
}  // namespace tint
