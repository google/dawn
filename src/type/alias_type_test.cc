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

#include "src/type/alias_type.h"

#include <memory>
#include <utility>

#include "src/ast/storage_class.h"
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
#include "src/type/struct_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace type {
namespace {

using AliasTest = TestHelper;

TEST_F(AliasTest, Create) {
  auto* a = ty.alias("a_type", ty.u32());
  EXPECT_EQ(a->symbol(), Symbol(1));
  EXPECT_EQ(a->type(), ty.u32());
}

TEST_F(AliasTest, Is) {
  auto* at = ty.alias("a", ty.i32());
  type::Type* ty = at;
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
  auto* at = ty.alias("Particle", ty.i32());
  EXPECT_EQ(at->type_name(), "__alias_tint_symbol_1__i32");
}

TEST_F(AliasTest, FriendlyName) {
  auto* at = ty.alias("Particle", ty.i32());
  EXPECT_EQ(at->FriendlyName(Symbols()), "Particle");
}

TEST_F(AliasTest, UnwrapIfNeeded_Alias) {
  auto* a = ty.alias("a_type", ty.u32());
  EXPECT_EQ(a->symbol(), Symbol(1));
  EXPECT_EQ(a->type(), ty.u32());
  EXPECT_EQ(a->UnwrapIfNeeded(), ty.u32());
  EXPECT_EQ(ty.u32()->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_AccessControl) {
  AccessControl a{ast::AccessControl::kReadOnly, ty.u32()};
  EXPECT_EQ(a.type(), ty.u32());
  EXPECT_EQ(a.UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel) {
  auto* a = ty.alias("a_type", ty.u32());
  auto* aa = ty.alias("aa_type", a);

  EXPECT_EQ(aa->symbol(), Symbol(2));
  EXPECT_EQ(aa->type(), a);
  EXPECT_EQ(aa->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel_AliasAccessControl) {
  auto* a = ty.alias("a_type", ty.u32());

  AccessControl aa{ast::AccessControl::kReadWrite, a};
  EXPECT_EQ(aa.type(), a);
  EXPECT_EQ(aa.UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_TwiceAliasPointerTwiceAlias) {
  auto* a = ty.alias("a_type", ty.u32());
  auto* aa = ty.alias("aa_type", a);
  Pointer paa{aa, ast::StorageClass::kUniform};
  auto* apaa = ty.alias("paa_type", &paa);
  auto* aapaa = ty.alias("aapaa_type", apaa);

  EXPECT_EQ(aapaa->symbol(), Symbol(4));
  EXPECT_EQ(aapaa->type(), apaa);
  EXPECT_EQ(aapaa->UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_SecondConsecutivePointerBlocksUnrapping) {
  auto* a = ty.alias("a_type", ty.u32());
  auto* aa = ty.alias("aa_type", a);

  Pointer paa{aa, ast::StorageClass::kUniform};
  Pointer ppaa{&paa, ast::StorageClass::kUniform};
  auto* appaa = ty.alias("appaa_type", &ppaa);
  EXPECT_EQ(appaa->UnwrapAll(), &paa);
}

TEST_F(AliasTest, UnwrapAll_SecondNonConsecutivePointerBlocksUnrapping) {
  auto* a = ty.alias("a_type", ty.u32());
  auto* aa = ty.alias("aa_type", a);
  Pointer paa{aa, ast::StorageClass::kUniform};

  auto* apaa = ty.alias("apaa_type", &paa);
  auto* aapaa = ty.alias("aapaa_type", apaa);
  Pointer paapaa{aapaa, ast::StorageClass::kUniform};
  auto* apaapaa = ty.alias("apaapaa_type", &paapaa);

  EXPECT_EQ(apaapaa->UnwrapAll(), &paa);
}

TEST_F(AliasTest, UnwrapAll_AccessControlPointer) {
  AccessControl a{ast::AccessControl::kReadOnly, ty.u32()};
  Pointer pa{&a, ast::StorageClass::kUniform};
  EXPECT_EQ(pa.type(), &a);
  EXPECT_EQ(pa.UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_PointerAccessControl) {
  Pointer p{ty.u32(), ast::StorageClass::kUniform};
  AccessControl a{ast::AccessControl::kReadOnly, &p};

  EXPECT_EQ(a.type(), &p);
  EXPECT_EQ(a.UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, MinBufferBindingSizeU32) {
  auto* alias = ty.alias("alias", ty.u32());
  EXPECT_EQ(4u, alias->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeArray) {
  Array array(ty.u32(), 4,
              ast::ArrayDecorationList{
                  create<ast::StrideDecoration>(4),
              });
  auto* alias = ty.alias("alias", &array);
  EXPECT_EQ(16u, alias->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeRuntimeArray) {
  Array array(ty.u32(), 0,
              ast::ArrayDecorationList{
                  create<ast::StrideDecoration>(4),
              });
  auto* alias = ty.alias("alias", &array);
  EXPECT_EQ(4u, alias->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, MinBufferBindingSizeStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)})},
      ast::StructDecorationList{});
  auto* struct_type = ty.struct_("struct_type", str);
  auto* alias = ty.alias("alias", struct_type);

  EXPECT_EQ(16u, alias->MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, alias->MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(AliasTest, BaseAlignmentU32) {
  auto* alias = ty.alias("alias", ty.u32());
  EXPECT_EQ(4u, alias->BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentArray) {
  Array array(ty.u32(), 4,
              ast::ArrayDecorationList{
                  create<ast::StrideDecoration>(4),
              });
  auto* alias = ty.alias("alias", &array);
  EXPECT_EQ(16u, alias->BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentRuntimeArray) {
  Array array(ty.u32(), 0,
              ast::ArrayDecorationList{
                  create<ast::StrideDecoration>(4),
              });
  auto* alias = ty.alias("alias", &array);
  EXPECT_EQ(16u, alias->BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(AliasTest, BaseAlignmentStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("foo", ty.u32(), {MemberOffset(0)}),
                            Member("bar", ty.u32(), {MemberOffset(4)})},
      ast::StructDecorationList{});
  auto* struct_type = ty.struct_("struct_type", str);
  auto* alias = ty.alias("alias", struct_type);

  EXPECT_EQ(16u, alias->BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, alias->BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(AliasTest, UnwrapAliasIfNeeded) {
  auto* alias1 = ty.alias("alias1", ty.f32());
  auto* alias2 = ty.alias("alias2", alias1);
  auto* alias3 = ty.alias("alias3", alias2);
  EXPECT_EQ(alias3->UnwrapAliasIfNeeded(), ty.f32());
}

}  // namespace
}  // namespace type
}  // namespace tint
