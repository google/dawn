// Copyright 2020 The Tint Authors.
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

#include "src/type/struct_type.h"

#include <utility>

#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace type {
namespace {

using StructTypeTest = TestHelper;

TEST_F(StructTypeTest, Creation) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::StructDecorationList{});
  auto* ptr = impl;
  auto* s = ty.struct_("S", impl);
  EXPECT_EQ(s->impl(), ptr);
}

TEST_F(StructTypeTest, Is) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::StructDecorationList{});
  auto* s = ty.struct_("S", impl);
  type::Type* ty = s;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_TRUE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(StructTypeTest, TypeName) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::StructDecorationList{});
  auto* s = ty.struct_("my_struct", impl);
  EXPECT_EQ(s->type_name(), "__struct_tint_symbol_1");
}

TEST_F(StructTypeTest, FriendlyName) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::StructDecorationList{});
  auto* s = ty.struct_("my_struct", impl);
  EXPECT_EQ(s->FriendlyName(Symbols()), "my_struct");
}

TEST_F(StructTypeTest, MinBufferBindingSize) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeArray) {
  Array arr(ty.u32(), 4,
            ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)}),
                            Member("bar", &arr, {MemberOffset(8)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(32u, s_ty->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(24u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeRuntimeArray) {
  Array arr(ty.u32(), 0,
            ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)}),
                            Member("bar", ty.u32(), {MemberOffset(8)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(12u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec2) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec2<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec3) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec3<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec4) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec4<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, s_ty->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignment) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(8)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentArray) {
  Array arr(ty.u32(), 4,
            ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)}),
                            Member("bar", &arr, {MemberOffset(8)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentRuntimeArray) {
  Array arr(ty.u32(), 0,
            ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)}),
                            Member("bar", ty.u32(), {MemberOffset(8)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(4u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec2) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec2<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec3) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec3<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec4) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.vec4<u32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s_ty = ty.struct_("s_ty", str);

  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, s_ty->BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace tint
