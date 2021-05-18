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

}  // namespace
}  // namespace resolver
}  // namespace tint
