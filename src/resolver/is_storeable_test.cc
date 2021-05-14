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
  EXPECT_FALSE(r()->IsStorable(ty.void_()));
}

TEST_F(ResolverIsStorableTest, Scalar) {
  EXPECT_TRUE(r()->IsStorable(ty.bool_()));
  EXPECT_TRUE(r()->IsStorable(ty.i32()));
  EXPECT_TRUE(r()->IsStorable(ty.u32()));
  EXPECT_TRUE(r()->IsStorable(ty.f32()));
}

TEST_F(ResolverIsStorableTest, Vector) {
  EXPECT_TRUE(r()->IsStorable(ty.vec2<i32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec3<i32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec4<i32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec2<u32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec3<u32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec4<u32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec2<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec3<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec4<f32>()));
}

TEST_F(ResolverIsStorableTest, Matrix) {
  EXPECT_TRUE(r()->IsStorable(ty.mat2x2<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat2x3<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat2x4<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x2<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x3<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x4<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x2<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x3<f32>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x4<f32>()));
}

TEST_F(ResolverIsStorableTest, Pointer) {
  EXPECT_FALSE(r()->IsStorable(ty.pointer<i32>(ast::StorageClass::kPrivate)));
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
  auto ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  Structure("S", {
                     Member("a", ty.i32()),
                     Member("b", ptr_ty),
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
  auto ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  auto* non_storable = Structure("nonstorable", {
                                                    Member("a", ty.i32()),
                                                    Member("b", ptr_ty),
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
