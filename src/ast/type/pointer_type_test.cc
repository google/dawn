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

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using PointerTypeTest = testing::Test;

TEST_F(PointerTypeTest, Creation) {
  I32Type i32;
  PointerType p{&i32, StorageClass::kStorageBuffer};
  EXPECT_EQ(p.type(), &i32);
  EXPECT_EQ(p.storage_class(), StorageClass::kStorageBuffer);
}

TEST_F(PointerTypeTest, Is) {
  I32Type i32;
  PointerType p{&i32, StorageClass::kFunction};
  EXPECT_FALSE(p.IsAlias());
  EXPECT_FALSE(p.IsArray());
  EXPECT_FALSE(p.IsBool());
  EXPECT_FALSE(p.IsF32());
  EXPECT_FALSE(p.IsI32());
  EXPECT_FALSE(p.IsMatrix());
  EXPECT_TRUE(p.IsPointer());
  EXPECT_FALSE(p.IsSampler());
  EXPECT_FALSE(p.IsStruct());
  EXPECT_FALSE(p.IsTexture());
  EXPECT_FALSE(p.IsU32());
  EXPECT_FALSE(p.IsVector());
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
