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
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using AliasTest = TestHelper;

TEST_F(AliasTest, Create) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  EXPECT_EQ(a.symbol(), Symbol(1));
  // EXPECT_EQ(a.name(), "a_type");
  EXPECT_EQ(a.type(), &u32);
}

TEST_F(AliasTest, Is) {
  I32 i32;

  Alias at{mod.RegisterSymbol("a"), "a", &i32};
  Type* ty = &at;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_TRUE(ty->Is<Alias>());
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

TEST_F(AliasTest, TypeName) {
  I32 i32;
  Alias at{mod.RegisterSymbol("Particle"), "Particle", &i32};
  EXPECT_EQ(at.type_name(), "__alias_tint_symbol_1__i32");
}

TEST_F(AliasTest, UnwrapIfNeeded_Alias) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  EXPECT_EQ(a.symbol(), Symbol(1));
  // EXPECT_EQ(a.name(), "a_type");
  EXPECT_EQ(a.type(), &u32);
  EXPECT_EQ(a.UnwrapIfNeeded(), &u32);
  EXPECT_EQ(u32.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTest, UnwrapIfNeeded_AccessControl) {
  U32 u32;
  AccessControl a{ast::AccessControl::kReadOnly, &u32};
  EXPECT_EQ(a.type(), &u32);
  EXPECT_EQ(a.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  Alias aa{mod.RegisterSymbol("aa_type"), "aa_type", &a};
  EXPECT_EQ(aa.symbol(), Symbol(2));
  // EXPECT_EQ(aa.name(), "aa_type");
  EXPECT_EQ(aa.type(), &a);
  EXPECT_EQ(aa.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel_AliasAccessControl) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  AccessControl aa{ast::AccessControl::kReadWrite, &a};
  EXPECT_EQ(aa.type(), &a);
  EXPECT_EQ(aa.UnwrapIfNeeded(), &u32);
}

TEST_F(AliasTest, UnwrapAll_TwiceAliasPointerTwiceAlias) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  Alias aa{mod.RegisterSymbol("aa_type"), "aa_type", &a};
  Pointer paa{&aa, StorageClass::kUniform};
  Alias apaa{mod.RegisterSymbol("paa_type"), "paa_type", &paa};
  Alias aapaa{mod.RegisterSymbol("aapaa_type"), "aapaa_type", &apaa};
  EXPECT_EQ(aapaa.symbol(), Symbol(4));
  // EXPECT_EQ(aapaa.name(), "aapaa_type");
  EXPECT_EQ(aapaa.type(), &apaa);
  EXPECT_EQ(aapaa.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTest, UnwrapAll_SecondConsecutivePointerBlocksUnrapping) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  Alias aa{mod.RegisterSymbol("aa_type"), "aa_type", &a};
  Pointer paa{&aa, StorageClass::kUniform};
  Pointer ppaa{&paa, StorageClass::kUniform};
  Alias appaa{mod.RegisterSymbol("appaa_type"), "appaa_type", &ppaa};
  EXPECT_EQ(appaa.UnwrapAll(), &paa);
}

TEST_F(AliasTest, UnwrapAll_SecondNonConsecutivePointerBlocksUnrapping) {
  U32 u32;
  Alias a{mod.RegisterSymbol("a_type"), "a_type", &u32};
  Alias aa{mod.RegisterSymbol("aa_type"), "aa_type", &a};
  Pointer paa{&aa, StorageClass::kUniform};
  Alias apaa{mod.RegisterSymbol("apaa_type"), "apaa_type", &paa};
  Alias aapaa{mod.RegisterSymbol("aapaa_type"), "aapaa_type", &apaa};
  Pointer paapaa{&aapaa, StorageClass::kUniform};
  Alias apaapaa{mod.RegisterSymbol("apaapaa_type"), "apaapaa_type", &paapaa};
  EXPECT_EQ(apaapaa.UnwrapAll(), &paa);
}

TEST_F(AliasTest, UnwrapAll_AccessControlPointer) {
  U32 u32;
  AccessControl a{ast::AccessControl::kReadOnly, &u32};
  Pointer pa{&a, StorageClass::kUniform};
  EXPECT_EQ(pa.type(), &a);
  EXPECT_EQ(pa.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTest, UnwrapAll_PointerAccessControl) {
  U32 u32;
  Pointer p{&u32, StorageClass::kUniform};
  AccessControl a{ast::AccessControl::kReadOnly, &p};
  EXPECT_EQ(a.type(), &p);
  EXPECT_EQ(a.UnwrapAll(), &u32);
  EXPECT_EQ(u32.UnwrapAll(), &u32);
}

TEST_F(AliasTest, MinBufferBindingSizeU32) {
  U32 u32;
  Alias alias{mod.RegisterSymbol("alias"), "alias", &u32};
  EXPECT_EQ(4u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeArray) {
  U32 u32;
  Array array(&u32, 4,
              ArrayDecorationList{
                  create<StrideDecoration>(4, Source{}),
              });
  Alias alias{mod.RegisterSymbol("alias"), "alias", &array};
  EXPECT_EQ(16u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeRuntimeArray) {
  U32 u32;
  Array array(&u32, 0,
              ArrayDecorationList{
                  create<StrideDecoration>(4, Source{}),
              });
  Alias alias{mod.RegisterSymbol("alias"), "alias", &array};
  EXPECT_EQ(4u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeStruct) {
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

  auto* str = create<ast::Struct>(Source{}, members, decos);
  Struct struct_type(mod.RegisterSymbol("struct_type"), "struct_type", str);
  Alias alias{mod.RegisterSymbol("alias"), "alias", &struct_type};
  EXPECT_EQ(16u, alias.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, alias.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(AliasTest, BaseAlignmentU32) {
  U32 u32;
  Alias alias{mod.RegisterSymbol("alias"), "alias", &u32};
  EXPECT_EQ(4u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentArray) {
  U32 u32;
  Array array(&u32, 4,
              ArrayDecorationList{
                  create<StrideDecoration>(4, Source{}),
              });
  Alias alias{mod.RegisterSymbol("alias"), "alias", &array};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentRuntimeArray) {
  U32 u32;
  Array array(&u32, 0,
              ArrayDecorationList{
                  create<StrideDecoration>(4, Source{}),
              });
  Alias alias{mod.RegisterSymbol("alias"), "alias", &array};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentStruct) {
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

  auto* str = create<ast::Struct>(Source{}, members, decos);
  Struct struct_type(mod.RegisterSymbol("struct_type"), "struct_type", str);
  Alias alias{mod.RegisterSymbol("alias"), "alias", &struct_type};
  EXPECT_EQ(16u, alias.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, alias.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
