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

#include "src/ast/type/u32_type.h"

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using U32TypeTest = TestHelper;

TEST_F(U32TypeTest, Is) {
  U32Type u;
  EXPECT_FALSE(u.IsAccessControl());
  EXPECT_FALSE(u.IsAlias());
  EXPECT_FALSE(u.IsArray());
  EXPECT_FALSE(u.IsBool());
  EXPECT_FALSE(u.IsF32());
  EXPECT_FALSE(u.IsI32());
  EXPECT_FALSE(u.IsMatrix());
  EXPECT_FALSE(u.IsPointer());
  EXPECT_FALSE(u.IsSampler());
  EXPECT_FALSE(u.IsStruct());
  EXPECT_FALSE(u.IsTexture());
  EXPECT_TRUE(u.IsU32());
  EXPECT_FALSE(u.IsVector());
}

TEST_F(U32TypeTest, TypeName) {
  U32Type u;
  EXPECT_EQ(u.type_name(), "__u32");
}

TEST_F(U32TypeTest, MinBufferBindingSize) {
  U32Type u;
  EXPECT_EQ(4u, u.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(U32TypeTest, BaseAlignment) {
  U32Type u;
  EXPECT_EQ(4u, u.BaseAlignment(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
