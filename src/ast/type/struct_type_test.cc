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

using StructTypeTest = TestHelper;

TEST_F(StructTypeTest, Creation) {
  auto* impl = create<Struct>();
  auto* ptr = impl;
  StructType s{"S", impl};
  EXPECT_EQ(s.impl(), ptr);
}

TEST_F(StructTypeTest, Is) {
  auto* impl = create<Struct>();
  StructType s{"S", impl};
  Type* ty = &s;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->Is<SamplerType>());
  EXPECT_TRUE(ty->Is<StructType>());
  EXPECT_FALSE(ty->Is<TextureType>());
  EXPECT_FALSE(ty->Is<U32Type>());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(StructTypeTest, TypeName) {
  auto* impl = create<Struct>();
  StructType s{"my_struct", impl};
  EXPECT_EQ(s.type_name(), "__struct_my_struct");
}

TEST_F(StructTypeTest, MinBufferBindingSize) {
  U32Type u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeArray) {
  U32Type u32;
  ArrayType arr(&u32, 4);
  {
    ArrayDecorationList decos;
    decos.push_back(create<StrideDecoration>(4, Source{}));
    arr.set_decorations(decos);
  }

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>("bar", &arr, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(32u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(24u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeRuntimeArray) {
  U32Type u32;
  ArrayType arr(&u32);
  {
    ArrayDecorationList decos;
    decos.push_back(create<StrideDecoration>(4, Source{}));
    arr.set_decorations(decos);
  }

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(12u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec2) {
  U32Type u32;
  VectorType vec2(&u32, 2);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec2, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec3) {
  U32Type u32;
  VectorType vec3(&u32, 3);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec3, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, MinBufferBindingSizeVec4) {
  U32Type u32;
  VectorType vec4(&u32, 4);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec4, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u,
            struct_type.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignment) {
  U32Type u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentArray) {
  U32Type u32;
  ArrayType arr(&u32, 4);
  {
    ArrayDecorationList decos;
    decos.push_back(create<StrideDecoration>(4, Source{}));
    arr.set_decorations(decos);
  }

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>("bar", &arr, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentRuntimeArray) {
  U32Type u32;
  ArrayType arr(&u32);
  {
    ArrayDecorationList decos;
    decos.push_back(create<StrideDecoration>(4, Source{}));
    arr.set_decorations(decos);
  }

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(8, Source{}));
    members.push_back(create<StructMember>("bar", &u32, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(4u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec2) {
  U32Type u32;
  VectorType vec2(&u32, 2);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec2, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec3) {
  U32Type u32;
  VectorType vec3(&u32, 3);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec3, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(StructTypeTest, BaseAlignmentVec4) {
  U32Type u32;
  VectorType vec4(&u32, 4);

  StructMemberList members;
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &vec4, deco));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, struct_type.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
