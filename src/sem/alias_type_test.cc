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

#include "src/sem/access_control_type.h"
#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using AliasTest = TestHelper;

TEST_F(AliasTest, Create) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  EXPECT_EQ(a->symbol(), Symbol(1, ID()));
  EXPECT_EQ(a->type(), ty.u32());
}

TEST_F(AliasTest, Is) {
  auto* at = create<Alias>(Sym("a"), ty.i32());
  sem::Type* ty = at;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_TRUE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<StructType>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(AliasTest, TypeName) {
  auto* at = create<Alias>(Sym("Particle"), ty.i32());
  EXPECT_EQ(at->type_name(), "__alias_$1__i32");
}

TEST_F(AliasTest, FriendlyName) {
  auto* at = create<Alias>(Sym("Particle"), ty.i32());
  EXPECT_EQ(at->FriendlyName(Symbols()), "Particle");
}

TEST_F(AliasTest, UnwrapIfNeeded_Alias) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  EXPECT_EQ(a->symbol(), Symbol(1, ID()));
  EXPECT_EQ(a->type(), ty.u32());
  EXPECT_EQ(a->UnwrapIfNeeded(), ty.u32());
  EXPECT_EQ(ty.u32()->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_AccessControl) {
  auto* a = create<AccessControl>(ast::AccessControl::kReadOnly, ty.u32());
  EXPECT_EQ(a->type(), ty.u32());
  EXPECT_EQ(a->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  auto* aa = create<Alias>(Sym("aa_type"), a);

  EXPECT_EQ(aa->symbol(), Symbol(2, ID()));
  EXPECT_EQ(aa->type(), a);
  EXPECT_EQ(aa->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapIfNeeded_MultiLevel_AliasAccessControl) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  auto* aa = create<AccessControl>(ast::AccessControl::kReadWrite, a);
  EXPECT_EQ(aa->type(), a);
  EXPECT_EQ(aa->UnwrapIfNeeded(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_TwiceAliasPointerTwiceAlias) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym(Sym("a_type")), u32);
  auto* aa = create<Alias>(Sym("aa_type"), a);
  auto* paa = create<Pointer>(aa, ast::StorageClass::kUniform);
  auto* apaa = create<Alias>(Sym("paa_type"), paa);
  auto* aapaa = create<Alias>(Sym("aapaa_type"), apaa);

  EXPECT_EQ(aapaa->symbol(), Symbol(4, ID()));
  EXPECT_EQ(aapaa->type(), apaa);
  EXPECT_EQ(aapaa->UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_SecondConsecutivePointerBlocksUnrapping) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  auto* aa = create<Alias>(Sym("aa_type"), a);

  auto* paa = create<Pointer>(aa, ast::StorageClass::kUniform);
  auto* ppaa = create<Pointer>(paa, ast::StorageClass::kUniform);
  auto* appaa = create<Alias>(Sym("appaa_type"), ppaa);
  EXPECT_EQ(appaa->UnwrapAll(), paa);
}

TEST_F(AliasTest, UnwrapAll_SecondNonConsecutivePointerBlocksUnrapping) {
  auto* a = create<Alias>(Sym("a_type"), ty.u32());
  auto* aa = create<Alias>(Sym("aa_type"), a);
  auto* paa = create<Pointer>(aa, ast::StorageClass::kUniform);

  auto* apaa = create<Alias>(Sym("apaa_type"), paa);
  auto* aapaa = create<Alias>(Sym("aapaa_type"), apaa);
  auto* paapaa = create<Pointer>(aapaa, ast::StorageClass::kUniform);
  auto* apaapaa = create<Alias>(Sym("apaapaa_type"), paapaa);

  EXPECT_EQ(apaapaa->UnwrapAll(), paa);
}

TEST_F(AliasTest, UnwrapAll_AccessControlPointer) {
  auto* a = create<AccessControl>(ast::AccessControl::kReadOnly, ty.u32());
  auto* pa = create<Pointer>(a, ast::StorageClass::kUniform);
  EXPECT_EQ(pa->type(), a);
  EXPECT_EQ(pa->UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, UnwrapAll_PointerAccessControl) {
  auto* p = create<Pointer>(ty.u32(), ast::StorageClass::kUniform);
  auto* a = create<AccessControl>(ast::AccessControl::kReadOnly, p);

  EXPECT_EQ(a->type(), p);
  EXPECT_EQ(a->UnwrapAll(), ty.u32());
}

TEST_F(AliasTest, UnwrapAliasIfNeeded) {
  auto* alias1 = create<Alias>(Sym("alias1"), ty.f32());
  auto* alias2 = create<Alias>(Sym("alias2"), alias1);
  auto* alias3 = create<Alias>(Sym("alias3"), alias2);
  EXPECT_EQ(alias3->UnwrapAliasIfNeeded(), ty.f32());
}

}  // namespace
}  // namespace sem
}  // namespace tint
