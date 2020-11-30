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

#include "src/ast/type/pointer_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using PointerTypeTest = TestHelper;

TEST_F(PointerTypeTest, Creation) {
  I32Type i32;
  PointerType p{&i32, StorageClass::kStorageBuffer};
  EXPECT_EQ(p.type(), &i32);
  EXPECT_EQ(p.storage_class(), StorageClass::kStorageBuffer);
}

TEST_F(PointerTypeTest, Is) {
  I32Type i32;
  PointerType p{&i32, StorageClass::kFunction};
  Type* ty = &p;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->IsF32());
  EXPECT_FALSE(ty->IsI32());
  EXPECT_FALSE(ty->IsMatrix());
  EXPECT_TRUE(ty->IsPointer());
  EXPECT_FALSE(ty->IsSampler());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_FALSE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(PointerTypeTest, TypeName) {
  I32Type i32;
  PointerType p{&i32, StorageClass::kWorkgroup};
  EXPECT_EQ(p.type_name(), "__ptr_workgroup__i32");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
