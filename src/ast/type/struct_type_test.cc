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

#include "src/ast/type/struct_type.h"

#include <utility>

#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  StructMemberList members;
  auto* impl = create<ast::Struct>(members);
  auto* ptr = impl;
  Struct s{"S", impl};
  EXPECT_EQ(s.impl(), ptr);
}

TEST_F(StructTest, Is) {
  StructMemberList members;
  auto* impl = create<ast::Struct>(members);
  Struct s{"S", impl};
  Type* ty = &s;
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

TEST_F(StructTest, TypeName) {
  StructMemberList members;
  auto* impl = create<ast::Struct>(members);
  Struct s{"my_struct", impl};
  EXPECT_EQ(s.type_name(), "__struct_my_struct");
}

TEST_F(StructTest, MinBufferBindingSize) {
  U32 u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, MinBufferBindingSizeArray) {
  U32 u32;
  Array arr(&u32, 4,
            ArrayDecorationList{create<StrideDecoration>(4, Source{})});

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &arr, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(32u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(24u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, MinBufferBindingSizeRuntimeArray) {
  U32 u32;
  Array arr(&u32, 0,
            ArrayDecorationList{create<StrideDecoration>(4, Source{})});

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(12u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, MinBufferBindingSizeVec2) {
  U32 u32;
  Vector vec2(&u32, 2);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec2, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, MinBufferBindingSizeVec3) {
  U32 u32;
  Vector vec3(&u32, 3);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec3, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, MinBufferBindingSizeVec4) {
  U32 u32;
  Vector vec4(&u32, 4);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec4, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignment) {
  U32 u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignmentArray) {
  U32 u32;
  Array arr(&u32, 4,
            ArrayDecorationList{create<StrideDecoration>(4, Source{})});

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &arr, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignmentRuntimeArray) {
  U32 u32;
  Array arr(&u32, 0,
            ArrayDecorationList{create<StrideDecoration>(4, Source{})});

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>(Source{}, "bar", &u32, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignmentVec2) {
  U32 u32;
  Vector vec2(&u32, 2);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec2, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignmentVec3) {
  U32 u32;
  Vector vec3(&u32, 3);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec3, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTest, BaseAlignmentVec4) {
  U32 u32;
  Vector vec4(&u32, 4);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>(Source{}, "foo", &vec4, deco));
  }
  StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  Struct struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
