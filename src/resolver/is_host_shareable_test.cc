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

using ResolverIsHostShareable = ResolverTest;

TEST_F(ResolverIsHostShareable, Void) {
  EXPECT_FALSE(r()->IsHostShareable(ty.void_()));
}

TEST_F(ResolverIsHostShareable, Bool) {
  EXPECT_FALSE(r()->IsHostShareable(ty.bool_()));
}

TEST_F(ResolverIsHostShareable, NumericScalar) {
  EXPECT_TRUE(r()->IsHostShareable(ty.i32()));
  EXPECT_TRUE(r()->IsHostShareable(ty.u32()));
  EXPECT_TRUE(r()->IsHostShareable(ty.f32()));
}

TEST_F(ResolverIsHostShareable, NumericVector) {
  EXPECT_TRUE(r()->IsHostShareable(ty.vec2<i32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec3<i32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec4<i32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec2<u32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec3<u32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec4<u32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec2<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec3<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.vec4<f32>()));
}

TEST_F(ResolverIsHostShareable, BoolVector) {
  EXPECT_FALSE(r()->IsHostShareable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec4<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec4<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec2<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec3<bool>()));
  EXPECT_FALSE(r()->IsHostShareable(ty.vec4<bool>()));
}

TEST_F(ResolverIsHostShareable, Matrix) {
  EXPECT_TRUE(r()->IsHostShareable(ty.mat2x2<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat2x3<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat2x4<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat3x2<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat3x3<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat3x4<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat4x2<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat4x3<f32>()));
  EXPECT_TRUE(r()->IsHostShareable(ty.mat4x4<f32>()));
}

TEST_F(ResolverIsHostShareable, Pointer) {
  EXPECT_FALSE(
      r()->IsHostShareable(ty.pointer<i32>(ast::StorageClass::kPrivate)));
}

TEST_F(ResolverIsHostShareable, AliasVoid) {
  EXPECT_FALSE(r()->IsHostShareable(ty.alias("a", ty.void_())));
}

TEST_F(ResolverIsHostShareable, AliasI32) {
  EXPECT_TRUE(r()->IsHostShareable(ty.alias("a", ty.i32())));
}

TEST_F(ResolverIsHostShareable, AccessControlVoid) {
  EXPECT_FALSE(r()->IsHostShareable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.void_())));
}

TEST_F(ResolverIsHostShareable, AccessControlI32) {
  EXPECT_TRUE(r()->IsHostShareable(
      create<type::AccessControl>(ast::AccessControl::kReadOnly, ty.i32())));
}

TEST_F(ResolverIsHostShareable, ArraySizedOfHostShareable) {
  EXPECT_TRUE(r()->IsHostShareable(ty.array(ty.i32(), 5)));
}

TEST_F(ResolverIsHostShareable, ArrayUnsizedOfHostShareable) {
  EXPECT_TRUE(r()->IsHostShareable(ty.array<i32>()));
}

TEST_F(ResolverIsHostShareable, Struct_AllMembersHostShareable) {
  EXPECT_TRUE(r()->IsHostShareable(Structure("S", {
                                                      Member("a", ty.i32()),
                                                      Member("b", ty.f32()),
                                                  })));
}

TEST_F(ResolverIsHostShareable, Struct_SomeMembersNonHostShareable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->IsHostShareable(Structure("S", {
                                                       Member("a", ty.i32()),
                                                       Member("b", ptr_ty),
                                                   })));
}

TEST_F(ResolverIsHostShareable, Struct_NestedHostShareable) {
  auto* host_shareable = Structure("S", {
                                            Member("a", ty.i32()),
                                            Member("b", ty.f32()),
                                        });
  EXPECT_TRUE(
      r()->IsHostShareable(Structure("S", {
                                              Member("a", ty.i32()),
                                              Member("b", host_shareable),
                                          })));
}

TEST_F(ResolverIsHostShareable, Struct_NestedNonHostShareable) {
  auto* ptr_ty = ty.pointer<i32>(ast::StorageClass::kPrivate);
  auto* non_host_shareable =
      Structure("non_host_shareable", {
                                          Member("a", ty.i32()),
                                          Member("b", ptr_ty),
                                      });
  EXPECT_FALSE(
      r()->IsHostShareable(Structure("S", {
                                              Member("a", ty.i32()),
                                              Member("b", non_host_shareable),
                                          })));
}

}  // namespace
}  // namespace resolver
}  // namespace tint
