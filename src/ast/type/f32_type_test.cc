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

#include "src/ast/type/f32_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using F32TypeTest = TestHelper;

TEST_F(F32TypeTest, Is) {
  F32Type f;
  Type* ty = &f;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_TRUE(ty->IsF32());
  EXPECT_FALSE(ty->IsI32());
  EXPECT_FALSE(ty->IsMatrix());
  EXPECT_FALSE(ty->IsPointer());
  EXPECT_FALSE(ty->IsSampler());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_FALSE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(F32TypeTest, TypeName) {
  F32Type f;
  EXPECT_EQ(f.type_name(), "__f32");
}

TEST_F(F32TypeTest, MinBufferBindingSize) {
  F32Type f;
  EXPECT_EQ(4u, f.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(F32TypeTest, BaseAlignment) {
  F32Type f;
  EXPECT_EQ(4u, f.BaseAlignment(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
