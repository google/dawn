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

#include "src/type/access_control_type.h"

#include <memory>
#include <utility>

#include "src/ast/storage_class.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace type {
namespace {

using AccessControlTest = TestHelper;

TEST_F(AccessControlTest, Create) {
  U32 u32;
  AccessControl a{ast::AccessControl::kReadWrite, &u32};
  EXPECT_TRUE(a.IsReadWrite());
  EXPECT_EQ(a.type(), &u32);
}

TEST_F(AccessControlTest, Is) {
  I32 i32;

  AccessControl at{ast::AccessControl::kReadOnly, &i32};
  Type* ty = &at;
  EXPECT_TRUE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(AccessControlTest, AccessRead) {
  I32 i32;
  AccessControl at{ast::AccessControl::kReadOnly, &i32};
  EXPECT_TRUE(at.IsReadOnly());
  EXPECT_FALSE(at.IsWriteOnly());
  EXPECT_FALSE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_read_only__i32");
}

TEST_F(AccessControlTest, AccessWrite) {
  I32 i32;
  AccessControl at{ast::AccessControl::kWriteOnly, &i32};
  EXPECT_FALSE(at.IsReadOnly());
  EXPECT_TRUE(at.IsWriteOnly());
  EXPECT_FALSE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_write_only__i32");
}

TEST_F(AccessControlTest, AccessReadWrite) {
  I32 i32;
  AccessControl at{ast::AccessControl::kReadWrite, &i32};
  EXPECT_FALSE(at.IsReadOnly());
  EXPECT_FALSE(at.IsWriteOnly());
  EXPECT_TRUE(at.IsReadWrite());

  EXPECT_EQ(at.type_name(), "__access_control_read_write__i32");
}

TEST_F(AccessControlTest, MinBufferBindingSizeU32) {
  U32 u32;
  AccessControl at{ast::AccessControl::kReadOnly, &u32};
  EXPECT_EQ(4u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, MinBufferBindingSizeArray) {
  U32 u32;
  Array array(&u32, 4,
              ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  AccessControl at{ast::AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, MinBufferBindingSizeRuntimeArray) {
  U32 u32;
  Array array(&u32, 0,
              ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  AccessControl at{ast::AccessControl::kReadOnly, &array};
  EXPECT_EQ(4u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, MinBufferBindingSizeStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32, {MemberOffset(0)}),
                            Member("bar", ty.u32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* struct_type = ty.struct_("struct_type", str);
  AccessControl at{ast::AccessControl::kReadOnly, struct_type};
  EXPECT_EQ(16u, at.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, at.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(AccessControlTest, BaseAlignmentU32) {
  U32 u32;
  AccessControl at{ast::AccessControl::kReadOnly, &u32};
  EXPECT_EQ(4u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, BaseAlignmentArray) {
  U32 u32;
  Array array(&u32, 4,
              ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  AccessControl at{ast::AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, BaseAlignmentRuntimeArray) {
  U32 u32;
  Array array(&u32, 0,
              ast::ArrayDecorationList{create<ast::StrideDecoration>(4)});
  AccessControl at{ast::AccessControl::kReadOnly, &array};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AccessControlTest, BaseAlignmentStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32, {MemberOffset(0)}),
                            Member("bar", ty.u32, {MemberOffset(4)})},
      ast::StructDecorationList{});
  auto* struct_type = ty.struct_("struct_type", str);

  AccessControl at{ast::AccessControl::kReadOnly, struct_type};
  EXPECT_EQ(16u, at.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, at.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace tint
