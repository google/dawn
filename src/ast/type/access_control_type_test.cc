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
#include "src/ast/type/i32_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
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
  EXPECT_TRUE(at.IsAccessControl());
  EXPECT_FALSE(at.IsAlias());
  EXPECT_FALSE(at.IsArray());
  EXPECT_FALSE(at.IsBool());
  EXPECT_FALSE(at.IsF32());
  EXPECT_FALSE(at.IsI32());
  EXPECT_FALSE(at.IsMatrix());
  EXPECT_FALSE(at.IsPointer());
  EXPECT_FALSE(at.IsSampler());
  EXPECT_FALSE(at.IsStruct());
  EXPECT_FALSE(at.IsTexture());
  EXPECT_FALSE(at.IsU32());
  EXPECT_FALSE(at.IsVector());
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
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(4u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, MinBufferBindingSizeStruct) {
  U32Type u32;
  StructMemberList members;

  StructMemberDecorationList deco;
  deco.push_back(std::make_unique<StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(
      std::make_unique<StructMember>("foo", &u32, std::move(deco)));

  deco = StructMemberDecorationList();
  deco.push_back(std::make_unique<StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(
      std::make_unique<StructMember>("bar", &u32, std::move(deco)));

  ast::StructDecorationList decos;

  auto str =
      std::make_unique<ast::Struct>(std::move(decos), std::move(members));
  StructType struct_type("struct_type", std::move(str));
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
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AccessControlType at{AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTypeTest, BaseAlignmentStruct) {
  U32Type u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(std::make_unique<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(
        std::make_unique<StructMember>("foo", &u32, std::move(deco)));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(std::make_unique<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(
        std::make_unique<StructMember>("bar", &u32, std::move(deco)));
  }
  ast::StructDecorationList decos;

  auto str =
      std::make_unique<ast::Struct>(std::move(decos), std::move(members));
  StructType struct_type("struct_type", std::move(str));
  AccessControlType at{AccessControl::kReadOnly, &struct_type};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, at.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
