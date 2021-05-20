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

namespace tint {
namespace resolver {
namespace {

using ResolverIsStorableTest = ResolverTest;

TEST_F(ResolverIsStorableTest, Void) {
  EXPECT_FALSE(r()->IsStorable(create<sem::Void>()));
}

TEST_F(ResolverIsStorableTest, Scalar) {
  EXPECT_TRUE(r()->IsStorable(create<sem::Bool>()));
  EXPECT_TRUE(r()->IsStorable(create<sem::I32>()));
  EXPECT_TRUE(r()->IsStorable(create<sem::U32>()));
  EXPECT_TRUE(r()->IsStorable(create<sem::F32>()));
}

TEST_F(ResolverIsStorableTest, Vector) {
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::I32>(), 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::I32>(), 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::I32>(), 4)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::U32>(), 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::U32>(), 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::U32>(), 4)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::F32>(), 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::F32>(), 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Vector>(create<sem::F32>(), 4)));
}

TEST_F(ResolverIsStorableTest, Matrix) {
  auto* vec2 = create<sem::Vector>(create<sem::F32>(), 2);
  auto* vec3 = create<sem::Vector>(create<sem::F32>(), 3);
  auto* vec4 = create<sem::Vector>(create<sem::F32>(), 4);
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec2, 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec2, 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec2, 4)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec3, 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec3, 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec3, 4)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec4, 2)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec4, 3)));
  EXPECT_TRUE(r()->IsStorable(create<sem::Matrix>(vec4, 4)));
}

TEST_F(ResolverIsStorableTest, Pointer) {
  auto* ptr =
      create<sem::Pointer>(create<sem::I32>(), ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->IsStorable(ptr));
}

TEST_F(ResolverIsStorableTest, ArraySizedOfStorable) {
  auto* arr = create<sem::Array>(create<sem::I32>(), 5, 4, 20, 4, true);
  EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, ArrayUnsizedOfStorable) {
  auto* arr = create<sem::Array>(create<sem::I32>(), 0, 4, 4, 4, true);
  EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, Struct_AllMembersStorable) {
  Structure("S", {
                     Member("a", ty.i32()),
                     Member("b", ty.f32()),
                 });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_SomeMembersNonStorable) {
  Structure("S", {
                     Member("a", ty.i32()),
                     Member("b", ty.pointer<i32>(ast::StorageClass::kPrivate)),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(error: ptr<private, i32> cannot be used as the type of a structure member)");
}

TEST_F(ResolverIsStorableTest, Struct_NestedStorable) {
  auto* storable = Structure("Storable", {
                                             Member("a", ty.i32()),
                                             Member("b", ty.f32()),
                                         });
  Structure("S", {
                     Member("a", ty.i32()),
                     Member("b", storable),
                 });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_NestedNonStorable) {
  auto* non_storable =
      Structure("nonstorable",
                {
                    Member("a", ty.i32()),
                    Member("b", ty.pointer<i32>(ast::StorageClass::kPrivate)),
                });
  Structure("S", {
                     Member("a", ty.i32()),
                     Member("b", non_storable),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(error: ptr<private, i32> cannot be used as the type of a structure member)");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
