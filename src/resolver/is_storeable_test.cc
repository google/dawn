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
#include "src/type/access_control_type.h"

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
  EXPECT_TRUE(r()->IsStorable(ty.vec2<unsigned>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec3<unsigned>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec4<unsigned>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec2<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec3<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.vec4<float>()));
}

TEST_F(ResolverIsStorableTest, Matrix) {
  EXPECT_TRUE(r()->IsStorable(ty.mat2x2<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat2x3<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat2x4<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x2<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x3<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat3x4<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x2<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x3<float>()));
  EXPECT_TRUE(r()->IsStorable(ty.mat4x4<float>()));
}

TEST_F(ResolverIsStorableTest, Pointer) {
  EXPECT_FALSE(r()->IsStorable(ty.pointer<i32>(ast::StorageClass::kPrivate)));
}

TEST_F(ResolverIsStorableTest, AliasVoid) {
  EXPECT_FALSE(r()->IsStorable(ty.alias("a", ty.void_())));
}

TEST_F(ResolverIsStorableTest, AliasI32) {
  EXPECT_TRUE(r()->IsStorable(ty.alias("a", ty.i32())));
}

TEST_F(ResolverIsStorableTest, AccessControlVoid) {
  EXPECT_FALSE(r()->IsStorable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.void_())));
}

TEST_F(ResolverIsStorableTest, AccessControlI32) {
  EXPECT_TRUE(r()->IsStorable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.i32())));
}

TEST_F(ResolverIsStorableTest, ArraySizedOfStorable) {
  EXPECT_TRUE(r()->IsStorable(ty.array(ty.i32(), 5)));
}

TEST_F(ResolverIsStorableTest, ArrayUnsizedOfStorable) {
  EXPECT_TRUE(r()->IsStorable(ty.array<i32>()));
}

TEST_F(ResolverIsStorableTest, Struct_AllMembersStorable) {
  EXPECT_TRUE(r()->IsStorable(Structure("S", {
                                                 Member("a", ty.i32()),
                                                 Member("b", ty.f32()),
                                             })));
}

TEST_F(ResolverIsStorableTest, Struct_SomeMembersNonStorable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->IsStorable(Structure("S", {
                                                  Member("a", ty.i32()),
                                                  Member("b", ptr_ty),
                                              })));
}

TEST_F(ResolverIsStorableTest, Struct_NestedStorable) {
  auto* storable = Structure("S", {
                                      Member("a", ty.i32()),
                                      Member("b", ty.f32()),
                                  });
  EXPECT_TRUE(r()->IsStorable(Structure("S", {
                                                 Member("a", ty.i32()),
                                                 Member("b", storable),
                                             })));
}

TEST_F(ResolverIsStorableTest, Struct_NestedNonStorable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  auto* non_storable = Structure("nonstorable", {
                                                    Member("a", ty.i32()),
                                                    Member("b", ptr_ty),
                                                });
  EXPECT_FALSE(r()->IsStorable(Structure("S", {
                                                  Member("a", ty.i32()),
                                                  Member("b", non_storable),
                                              })));
}

}  // namespace
}  // namespace resolver
}  // namespace tint
