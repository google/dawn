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

using ResolverIsHostSharable = ResolverTest;

TEST_F(ResolverIsHostSharable, Void) {
  EXPECT_FALSE(r()->IsHostSharable(ty.void_()));
}

TEST_F(ResolverIsHostSharable, Bool) {
  EXPECT_FALSE(r()->IsHostSharable(ty.bool_()));
}

TEST_F(ResolverIsHostSharable, NumericScalar) {
  EXPECT_TRUE(r()->IsHostSharable(ty.i32()));
  EXPECT_TRUE(r()->IsHostSharable(ty.u32()));
  EXPECT_TRUE(r()->IsHostSharable(ty.f32()));
}

TEST_F(ResolverIsHostSharable, NumericVector) {
  EXPECT_TRUE(r()->IsHostSharable(ty.vec2<i32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec3<i32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec4<i32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec2<u32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec3<u32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec4<u32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec2<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec3<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.vec4<f32>()));
}

TEST_F(ResolverIsHostSharable, BoolVector) {
  EXPECT_FALSE(r()->IsHostSharable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec4<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec4<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostSharable(ty.vec4<bool>()));
}

TEST_F(ResolverIsHostSharable, Matrix) {
  EXPECT_TRUE(r()->IsHostSharable(ty.mat2x2<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat2x3<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat2x4<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat3x2<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat3x3<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat3x4<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat4x2<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat4x3<f32>()));
  EXPECT_TRUE(r()->IsHostSharable(ty.mat4x4<f32>()));
}

TEST_F(ResolverIsHostSharable, Pointer) {
  EXPECT_FALSE(
      r()->IsHostSharable(ty.pointer<i32>(ast::StorageClass::kPrivate)));
}

TEST_F(ResolverIsHostSharable, AliasVoid) {
  EXPECT_FALSE(r()->IsHostSharable(ty.alias("a", ty.void_())));
}

TEST_F(ResolverIsHostSharable, AliasI32) {
  EXPECT_TRUE(r()->IsHostSharable(ty.alias("a", ty.i32())));
}

TEST_F(ResolverIsHostSharable, AccessControlVoid) {
  EXPECT_FALSE(r()->IsHostSharable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.void_())));
}

TEST_F(ResolverIsHostSharable, AccessControlI32) {
  EXPECT_TRUE(r()->IsHostSharable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.i32())));
}

TEST_F(ResolverIsHostSharable, ArraySizedOfHostSharable) {
  EXPECT_TRUE(r()->IsHostSharable(ty.array(ty.i32(), 5)));
}

TEST_F(ResolverIsHostSharable, ArrayUnsizedOfHostSharable) {
  EXPECT_TRUE(r()->IsHostSharable(ty.array<i32>()));
}

TEST_F(ResolverIsHostSharable, Struct_AllMembersHostSharable) {
  EXPECT_TRUE(r()->IsHostSharable(Structure("S", {
                                                     Member("a", ty.i32()),
                                                     Member("b", ty.f32()),
                                                 })));
}

TEST_F(ResolverIsHostSharable, Struct_SomeMembersNonHostSharable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->IsHostSharable(Structure("S", {
                                                      Member("a", ty.i32()),
                                                      Member("b", ptr_ty),
                                                  })));
}

TEST_F(ResolverIsHostSharable, Struct_NestedHostSharable) {
  auto* host_sharable = Structure("S", {
                                           Member("a", ty.i32()),
                                           Member("b", ty.f32()),
                                       });
  EXPECT_TRUE(r()->IsHostSharable(Structure("S", {
                                                     Member("a", ty.i32()),
                                                     Member("b", host_sharable),
                                                 })));
}

TEST_F(ResolverIsHostSharable, Struct_NestedNonHostSharable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  auto* non_host_sharable =
      Structure("non_host_sharable", {
                                         Member("a", ty.i32()),
                                         Member("b", ptr_ty),
                                     });
  EXPECT_FALSE(
      r()->IsHostSharable(Structure("S", {
                                             Member("a", ty.i32()),
                                             Member("b", non_host_sharable),
                                         })));
}

}  // namespace
}  // namespace resolver
}  // namespace tint
