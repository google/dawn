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

#include "gtest/gtest.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/u32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using AliasTypeTest = testing::Test;

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

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
