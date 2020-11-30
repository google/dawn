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

#include "src/ast/type/bool_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using BoolTypeTest = TestHelper;

TEST_F(BoolTypeTest, Is) {
  BoolType b;
  Type* ty = &b;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->IsAlias());
  EXPECT_FALSE(ty->IsArray());
  EXPECT_TRUE(ty->IsBool());
  EXPECT_FALSE(ty->IsF32());
  EXPECT_FALSE(ty->IsI32());
  EXPECT_FALSE(ty->IsMatrix());
  EXPECT_FALSE(ty->IsPointer());
  EXPECT_FALSE(ty->IsSampler());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_FALSE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(BoolTypeTest, TypeName) {
  BoolType b;
  EXPECT_EQ(b.type_name(), "__bool");
}

TEST_F(BoolTypeTest, MinBufferBindingSize) {
  BoolType b;
  EXPECT_EQ(0u, b.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
