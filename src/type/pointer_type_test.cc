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
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"

namespace tint {
namespace type {
namespace {

using PointerTest = TestHelper;

TEST_F(PointerTest, Creation) {
  I32 i32;
  Pointer p{&i32, ast::StorageClass::kStorage};
  EXPECT_EQ(p.type(), &i32);
  EXPECT_EQ(p.storage_class(), ast::StorageClass::kStorage);
}

TEST_F(PointerTest, Is) {
  I32 i32;
  Pointer p{&i32, ast::StorageClass::kFunction};
  Type* ty = &p;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_TRUE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(PointerTest, TypeName) {
  I32 i32;
  Pointer p{&i32, ast::StorageClass::kWorkgroup};
  EXPECT_EQ(p.type_name(), "__ptr_workgroup__i32");
}

TEST_F(PointerTest, FriendlyNameWithStorageClass) {
  Pointer p{ty.i32(), ast::StorageClass::kWorkgroup};
  EXPECT_EQ(p.FriendlyName(Symbols()), "ptr<workgroup, i32>");
}

TEST_F(PointerTest, FriendlyNameWithoutStorageClass) {
  Pointer p{ty.i32(), ast::StorageClass::kNone};
  EXPECT_EQ(p.FriendlyName(Symbols()), "ptr<i32>");
}

}  // namespace
}  // namespace type
}  // namespace tint
