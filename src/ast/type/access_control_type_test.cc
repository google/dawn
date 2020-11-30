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

#include "src/ast/type/access_control_type.h"

#include <memory>
#include <utility>

#include "src/ast/storage_class.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using AccessControlTypeTest = TestHelper;

TEST_F(AccessControlTypeTest, Create) {
  U32Type u32;
  AccessControlType a{AccessControl::kReadWrite, &u32};
  EXPECT_TRUE(a.IsReadWrite());
  EXPECT_EQ(a.type(), &u32);
}

TEST_F(AccessControlTypeTest, Is) {
  I32Type i32;

  AccessControlType at{AccessControl::kReadOnly, &i32};
  Type* ty = &at;
  EXPECT_TRUE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->Is<SamplerType>());
  EXPECT_FALSE(ty->Is<StructType>());
  EXPECT_FALSE(ty->Is<TextureType>());
  EXPECT_FALSE(ty->Is<U32Type>());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(AccessControlTypeTest, AccessRead) {
  I32Type i32;
  AccessControlType at{AccessControl::kReadOnly, &i32};
  EXPECT_TRUE(at.IsReadOnly());
  EXPECT_FALSE(at.IsWriteOnly());
  EXPECT_FALSE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_read_only__i32");
}

TEST_F(AccessControlTypeTest, AccessWrite) {
  I32Type i32;
  AccessControlType at{AccessControl::kWriteOnly, &i32};
  EXPECT_FALSE(at.IsReadOnly());
  EXPECT_TRUE(at.IsWriteOnly());
  EXPECT_FALSE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_write_only__i32");
}

TEST_F(AccessControlTypeTest, AccessReadWrite) {
  I32Type i32;
  AccessControlType at{AccessControl::kReadWrite, &i32};
  EXPECT_FALSE(at.IsReadOnly());
  EXPECT_FALSE(at.IsWriteOnly());
  EXPECT_TRUE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_read_write__i32");
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeU32) {
  U32Type u32;
  AccessControlType at{AccessControl::kReadOnly, &u32};
  EXPECT_EQ(4u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeArray) {
  U32Type u32;
  ArrayType array(&u32, 4);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(decos);
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(decos);
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(4u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeStruct) {
  U32Type u32;
  StructMemberList members;

  StructMemberDecorationList deco;
  deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<StructMember>("foo", &u32, deco));

  deco = StructMemberDecorationList();
  deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<StructMember>("bar", &u32, deco));

  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  StructType struct_type("struct_type", str);
  AccessControlType at{AccessControl::kReadOnly, &struct_type};
  EXPECT_EQ(16u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, at.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentU32) {
  U32Type u32;
  AccessControlType at{AccessControl::kReadOnly, &u32};
  EXPECT_EQ(4u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentArray) {
  U32Type u32;
  ArrayType array(&u32, 4);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(decos);
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(decos);
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentStruct) {
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
  AccessControlType at{AccessControl::kReadOnly, &struct_type};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, at.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
