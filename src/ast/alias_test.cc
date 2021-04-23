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

#include "src/ast/alias.h"
#include "src/ast/access_control.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/struct.h"
#include "src/ast/test_helper.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
namespace {

using AstAliasTest = TestHelper;

TEST_F(AstAliasTest, Create) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  EXPECT_EQ(a->symbol(), Symbol(1, ID()));
  EXPECT_EQ(a->type(), u32);
}

TEST_F(AstAliasTest, Is) {
  Type* ty = create<Alias>(Sym("a"), create<I32>());
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

// Check for linear-time evaluation of Alias::type_name().
// If type_name() is non-linear, this test should noticeably stall.
// See: crbug.com/1200936
TEST_F(AstAliasTest, TypeName_LinearTime) {
  Type* type = ty.i32();
  for (int i = 0; i < 1024; i++) {
    type = create<Alias>(Symbols().New(), type);
  }
  for (int i = 0; i < 16384; i++) {
    type->type_name();
  }
}

TEST_F(AstAliasTest, TypeName) {
  auto* at = create<Alias>(Sym("Particle"), create<I32>());
  EXPECT_EQ(at->type_name(), "__alias_$1__i32");
}

TEST_F(AstAliasTest, FriendlyName) {
  auto* at = create<Alias>(Sym("Particle"), create<I32>());
  EXPECT_EQ(at->FriendlyName(Symbols()), "Particle");
}

TEST_F(AstAliasTest, UnwrapIfNeeded_Alias) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  EXPECT_EQ(a->symbol(), Symbol(1, ID()));
  EXPECT_EQ(a->type(), u32);
  EXPECT_EQ(a->UnwrapIfNeeded(), u32);
  EXPECT_EQ(u32->UnwrapIfNeeded(), u32);
}

TEST_F(AstAliasTest, UnwrapIfNeeded_AccessControl) {
  auto* u32 = create<U32>();
  auto* ac = create<AccessControl>(AccessControl::kReadOnly, u32);
  EXPECT_EQ(ac->type(), u32);
  EXPECT_EQ(ac->UnwrapIfNeeded(), u32);
}

TEST_F(AstAliasTest, UnwrapIfNeeded_MultiLevel) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  auto* aa = create<Alias>(Sym("aa_type"), a);

  EXPECT_EQ(aa->symbol(), Symbol(2, ID()));
  EXPECT_EQ(aa->type(), a);
  EXPECT_EQ(aa->UnwrapIfNeeded(), u32);
}

TEST_F(AstAliasTest, UnwrapIfNeeded_MultiLevel_AliasAccessControl) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);

  auto* ac = create<AccessControl>(AccessControl::kReadWrite, a);
  EXPECT_EQ(ac->type(), a);
  EXPECT_EQ(ac->UnwrapIfNeeded(), u32);
}

TEST_F(AstAliasTest, UnwrapAll_TwiceAliasPointerTwiceAlias) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  auto* aa = create<Alias>(Sym("aa_type"), a);
  auto* paa = create<Pointer>(aa, StorageClass::kUniform);
  auto* apaa = create<Alias>(Sym("paa_type"), paa);
  auto* aapaa = create<Alias>(Sym("aapaa_type"), apaa);

  EXPECT_EQ(aapaa->symbol(), Symbol(4, ID()));
  EXPECT_EQ(aapaa->type(), apaa);
  EXPECT_EQ(aapaa->UnwrapAll(), u32);
}

TEST_F(AstAliasTest, UnwrapAll_SecondConsecutivePointerBlocksUnrapping) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  auto* aa = create<Alias>(Sym("aa_type"), a);

  auto* paa = create<Pointer>(aa, StorageClass::kUniform);
  auto* ppaa = create<Pointer>(paa, StorageClass::kUniform);
  auto* appaa = create<Alias>(Sym("appaa_type"), ppaa);
  EXPECT_EQ(appaa->UnwrapAll(), paa);
}

TEST_F(AstAliasTest, UnwrapAll_SecondNonConsecutivePointerBlocksUnrapping) {
  auto* u32 = create<U32>();
  auto* a = create<Alias>(Sym("a_type"), u32);
  auto* aa = create<Alias>(Sym("aa_type"), a);
  auto* paa = create<Pointer>(aa, StorageClass::kUniform);

  auto* apaa = create<Alias>(Sym("apaa_type"), paa);
  auto* aapaa = create<Alias>(Sym("aapaa_type"), apaa);
  auto* paapaa = create<Pointer>(aapaa, StorageClass::kUniform);
  auto* apaapaa = create<Alias>(Sym("apaapaa_type"), paapaa);

  EXPECT_EQ(apaapaa->UnwrapAll(), paa);
}

TEST_F(AstAliasTest, UnwrapAll_AccessControlPointer) {
  auto* u32 = create<U32>();
  auto* a = create<AccessControl>(AccessControl::kReadOnly, u32);
  auto* pa = create<Pointer>(a, StorageClass::kUniform);
  EXPECT_EQ(pa->type(), a);
  EXPECT_EQ(pa->UnwrapAll(), u32);
}

TEST_F(AstAliasTest, UnwrapAll_PointerAccessControl) {
  auto* u32 = create<U32>();
  auto* p = create<Pointer>(u32, StorageClass::kUniform);
  auto* a = create<AccessControl>(AccessControl::kReadOnly, p);

  EXPECT_EQ(a->type(), p);
  EXPECT_EQ(a->UnwrapAll(), u32);
}

TEST_F(AstAliasTest, UnwrapAliasIfNeeded) {
  auto* f32 = create<F32>();
  auto* alias1 = create<Alias>(Sym("alias1"), f32);
  auto* alias2 = create<Alias>(Sym("alias2"), alias1);
  auto* alias3 = create<Alias>(Sym("alias3"), alias2);
  EXPECT_EQ(alias3->UnwrapAliasIfNeeded(), f32);
}

}  // namespace
}  // namespace ast
}  // namespace tint
