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

#include "src/ast/type/alias_type.h"

#include <memory>
#include <utility>

#include "src/ast/storage_class.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using AliasTypeTest = TestHelper;

TEST_F(AliasTypeTest, Create) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  EXPECT_EQ(a.name(), "a_type");
  EXPECT_EQ(a.type(), &u32);
}

TEST_F(AliasTypeTest, Is) {
  I32Type i32;

  AliasType at{"a", &i32};
  EXPECT_FALSE(at.IsAccessControl());
  EXPECT_TRUE(at.IsAlias());
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

TEST_F(AliasTypeTest, TypeName) {
  I32Type i32;
  AliasType at{"Particle", &i32};
  EXPECT_EQ(at.type_name(), "__alias_Particle__i32");
}

TEST_F(AliasTypeTest, UnwrapIfNeeded_Alias) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  EXPECT_EQ(a.name(), "a_type");
  EXPECT_EQ(a.type(), &u32);
  EXPECT_EQ(a.UnwrapIfNeeded(), &u32);
  EXPECT_EQ(u32.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTypeTest, UnwrapIfNeeded_AccessControl) {
  U32Type u32;
  AccessControlType a{AccessControl::kReadOnly, &u32};
  EXPECT_EQ(a.type(), &u32);
  EXPECT_EQ(a.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTypeTest, UnwrapIfNeeded_MultiLevel) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  AliasType aa{"aa_type", &a};
  EXPECT_EQ(aa.name(), "aa_type");
  EXPECT_EQ(aa.type(), &a);
  EXPECT_EQ(aa.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTypeTest, UnwrapIfNeeded_MultiLevel_AliasAccessControl) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  AccessControlType aa{AccessControl::kReadWrite, &a};
  EXPECT_EQ(aa.type(), &a);
  EXPECT_EQ(aa.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTypeTest, UnwrapAll_TwiceAliasPointerTwiceAlias) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  AliasType aa{"aa_type", &a};
  PointerType paa{&aa, StorageClass::kUniform};
  AliasType apaa{"paa_type", &paa};
  AliasType aapaa{"aapaa_type", &apaa};
  EXPECT_EQ(aapaa.name(), "aapaa_type");
  EXPECT_EQ(aapaa.type(), &apaa);
  EXPECT_EQ(aapaa.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTypeTest, UnwrapAll_SecondConsecutivePointerBlocksUnrapping) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  AliasType aa{"aa_type", &a};
  PointerType paa{&aa, StorageClass::kUniform};
  PointerType ppaa{&paa, StorageClass::kUniform};
  AliasType appaa{"appaa_type", &ppaa};
  EXPECT_EQ(appaa.UnwrapAll(), &paa);
}

TEST_F(AliasTypeTest, UnwrapAll_SecondNonConsecutivePointerBlocksUnrapping) {
  U32Type u32;
  AliasType a{"a_type", &u32};
  AliasType aa{"aa_type", &a};
  PointerType paa{&aa, StorageClass::kUniform};
  AliasType apaa{"apaa_type", &paa};
  AliasType aapaa{"aapaa_type", &apaa};
  PointerType paapaa{&aapaa, StorageClass::kUniform};
  AliasType apaapaa{"apaapaa_type", &paapaa};
  EXPECT_EQ(apaapaa.UnwrapAll(), &paa);
}

TEST_F(AliasTypeTest, UnwrapAll_AccessControlPointer) {
  U32Type u32;
  AccessControlType a{AccessControl::kReadOnly, &u32};
  PointerType pa{&a, StorageClass::kUniform};
  EXPECT_EQ(pa.type(), &a);
  EXPECT_EQ(pa.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTypeTest, UnwrapAll_PointerAccessControl) {
  U32Type u32;
  PointerType p{&u32, StorageClass::kUniform};
  AccessControlType a{AccessControl::kReadOnly, &p};
  EXPECT_EQ(a.type(), &p);
  EXPECT_EQ(a.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTypeTest, MinBufferBindingSizeU32) {
  U32Type u32;
  AliasType alias{"alias", &u32};
  EXPECT_EQ(4u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, MinBufferBindingSizeArray) {
  U32Type u32;
  ArrayType array(&u32, 4);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AliasType alias{"alias", &array};
  EXPECT_EQ(16u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, MinBufferBindingSizeRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AliasType alias{"alias", &array};
  EXPECT_EQ(4u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, MinBufferBindingSizeStruct) {
  U32Type u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, std::move(deco)));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, std::move(deco)));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(std::move(decos), std::move(members));
  StructType struct_type("struct_type", std::move(str));
  AliasType alias{"alias", &struct_type};
  EXPECT_EQ(16u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, alias.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(AliasTypeTest, BaseAlignmentU32) {
  U32Type u32;
  AliasType alias{"alias", &u32};
  EXPECT_EQ(4u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, BaseAlignmentArray) {
  U32Type u32;
  ArrayType array(&u32, 4);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AliasType alias{"alias", &array};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, BaseAlignmentRuntimeArray) {
  U32Type u32;
  ArrayType array(&u32);
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));
  array.set_decorations(std::move(decos));
  AliasType alias{"alias", &array};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTypeTest, BaseAlignmentStruct) {
  U32Type u32;
  StructMemberList members;

  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(0, Source{}));
    members.push_back(create<StructMember>("foo", &u32, std::move(deco)));
  }
  {
    StructMemberDecorationList deco;
    deco.push_back(create<StructMemberOffsetDecoration>(4, Source{}));
    members.push_back(create<StructMember>("bar", &u32, std::move(deco)));
  }
  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(std::move(decos), std::move(members));
  StructType struct_type("struct_type", std::move(str));
  AliasType alias{"alias", &struct_type};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, alias.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
