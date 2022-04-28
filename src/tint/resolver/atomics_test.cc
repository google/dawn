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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/reference.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

struct ResolverAtomicTest : public resolver::TestHelper,
                            public testing::Test {};

TEST_F(ResolverAtomicTest, GlobalWorkgroupI32) {
  auto* g = Global("a", ty.atomic(Source{{12, 34}}, ty.i32()),
                   ast::StorageClass::kWorkgroup);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_TRUE(TypeOf(g)->Is<sem::Reference>());
  auto* atomic = TypeOf(g)->UnwrapRef()->As<sem::Atomic>();
  ASSERT_NE(atomic, nullptr);
  EXPECT_TRUE(atomic->Type()->Is<sem::I32>());
}

TEST_F(ResolverAtomicTest, GlobalWorkgroupU32) {
  auto* g = Global("a", ty.atomic(Source{{12, 34}}, ty.u32()),
                   ast::StorageClass::kWorkgroup);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_TRUE(TypeOf(g)->Is<sem::Reference>());
  auto* atomic = TypeOf(g)->UnwrapRef()->As<sem::Atomic>();
  ASSERT_NE(atomic, nullptr);
  EXPECT_TRUE(atomic->Type()->Is<sem::U32>());
}

TEST_F(ResolverAtomicTest, GlobalStorageStruct) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  auto* g = Global("g", ty.Of(s), ast::StorageClass::kStorage,
                   ast::Access::kReadWrite,
                   ast::AttributeList{
                       create<ast::BindingAttribute>(0),
                       create<ast::GroupAttribute>(0),
                   });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_TRUE(TypeOf(g)->Is<sem::Reference>());
  auto* str = TypeOf(g)->UnwrapRef()->As<sem::Struct>();
  ASSERT_NE(str, nullptr);
  ASSERT_EQ(str->Members().size(), 1u);
  auto* atomic = str->Members()[0]->Type()->As<sem::Atomic>();
  ASSERT_NE(atomic, nullptr);
  ASSERT_TRUE(atomic->Type()->Is<sem::I32>());
}

}  // namespace
}  // namespace tint::resolver
