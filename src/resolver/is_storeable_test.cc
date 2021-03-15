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
  auto* void_ty = ty.void_();

  EXPECT_FALSE(r()->IsStorable(void_ty));
}

TEST_F(ResolverIsStorableTest, Scalar) {
  auto* bool_ = ty.bool_();
  auto* i32 = ty.i32();
  auto* u32 = ty.u32();
  auto* f32 = ty.f32();

  EXPECT_TRUE(r()->IsStorable(bool_));
  EXPECT_TRUE(r()->IsStorable(i32));
  EXPECT_TRUE(r()->IsStorable(u32));
  EXPECT_TRUE(r()->IsStorable(f32));
}

TEST_F(ResolverIsStorableTest, Vector) {
  auto* vec2_i32 = ty.vec2<int>();
  auto* vec3_i32 = ty.vec3<int>();
  auto* vec4_i32 = ty.vec4<int>();
  auto* vec2_u32 = ty.vec2<unsigned>();
  auto* vec3_u32 = ty.vec3<unsigned>();
  auto* vec4_u32 = ty.vec4<unsigned>();
  auto* vec2_f32 = ty.vec2<float>();
  auto* vec3_f32 = ty.vec3<float>();
  auto* vec4_f32 = ty.vec4<float>();

  EXPECT_TRUE(r()->IsStorable(vec2_i32));
  EXPECT_TRUE(r()->IsStorable(vec3_i32));
  EXPECT_TRUE(r()->IsStorable(vec4_i32));
  EXPECT_TRUE(r()->IsStorable(vec2_u32));
  EXPECT_TRUE(r()->IsStorable(vec3_u32));
  EXPECT_TRUE(r()->IsStorable(vec4_u32));
  EXPECT_TRUE(r()->IsStorable(vec2_f32));
  EXPECT_TRUE(r()->IsStorable(vec3_f32));
  EXPECT_TRUE(r()->IsStorable(vec4_f32));
}

TEST_F(ResolverIsStorableTest, Matrix) {
  auto* mat2x2 = ty.mat2x2<float>();
  auto* mat2x3 = ty.mat2x3<float>();
  auto* mat2x4 = ty.mat2x4<float>();
  auto* mat3x2 = ty.mat3x2<float>();
  auto* mat3x3 = ty.mat3x3<float>();
  auto* mat3x4 = ty.mat3x4<float>();
  auto* mat4x2 = ty.mat4x2<float>();
  auto* mat4x3 = ty.mat4x3<float>();
  auto* mat4x4 = ty.mat4x4<float>();

  EXPECT_TRUE(r()->IsStorable(mat2x2));
  EXPECT_TRUE(r()->IsStorable(mat2x3));
  EXPECT_TRUE(r()->IsStorable(mat2x4));
  EXPECT_TRUE(r()->IsStorable(mat3x2));
  EXPECT_TRUE(r()->IsStorable(mat3x3));
  EXPECT_TRUE(r()->IsStorable(mat3x4));
  EXPECT_TRUE(r()->IsStorable(mat4x2));
  EXPECT_TRUE(r()->IsStorable(mat4x3));
  EXPECT_TRUE(r()->IsStorable(mat4x4));
}

TEST_F(ResolverIsStorableTest, Pointer) {
  auto* ptr_ty = ty.pointer<int>(ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->IsStorable(ptr_ty));
}

TEST_F(ResolverIsStorableTest, AliasVoid) {
  auto* alias = ty.alias("myalias", ty.void_());

  EXPECT_FALSE(r()->IsStorable(alias));
}

TEST_F(ResolverIsStorableTest, AliasI32) {
  auto* alias = ty.alias("myalias", ty.i32());

  EXPECT_TRUE(r()->IsStorable(alias));
}

TEST_F(ResolverIsStorableTest, ArraySizedOfStorable) {
  auto* arr = ty.array(ty.i32(), 5);

  EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, ArrayUnsizedOfStorable) {
  auto* arr = ty.array<int>();

  EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, Struct_AllMembersStorable) {
  ast::StructMemberList members{Member("a", ty.i32()), Member("b", ty.f32())};
  auto* s = create<ast::Struct>(Source{}, members, ast::DecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);

  EXPECT_TRUE(r()->IsStorable(s_ty));
}

TEST_F(ResolverIsStorableTest, Struct_SomeMembersNonStorable) {
  auto* ptr_ty = ty.pointer<int>(ast::StorageClass::kPrivate);
  ast::StructMemberList members{Member("a", ty.i32()), Member("b", ptr_ty)};
  auto* s = create<ast::Struct>(Source{}, members, ast::DecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);

  EXPECT_FALSE(r()->IsStorable(s_ty));
}

}  // namespace
}  // namespace resolver
}  // namespace tint
