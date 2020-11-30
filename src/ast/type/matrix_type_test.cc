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

#include "src/ast/type/matrix_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using MatrixTypeTest = TestHelper;

TEST_F(MatrixTypeTest, Creation) {
  I32Type i32;
  MatrixType m{&i32, 2, 4};
  EXPECT_EQ(m.type(), &i32);
  EXPECT_EQ(m.rows(), 2u);
  EXPECT_EQ(m.columns(), 4u);
}

TEST_F(MatrixTypeTest, Is) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  Type* ty = &m;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->IsAlias());
  EXPECT_FALSE(ty->IsArray());
  EXPECT_FALSE(ty->IsBool());
  EXPECT_FALSE(ty->IsF32());
  EXPECT_FALSE(ty->IsI32());
  EXPECT_TRUE(ty->IsMatrix());
  EXPECT_FALSE(ty->IsPointer());
  EXPECT_FALSE(ty->IsSampler());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_FALSE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(MatrixTypeTest, TypeName) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  EXPECT_EQ(m.type_name(), "__mat_2_3__i32");
}

TEST_F(MatrixTypeTest, MinBufferBindingSize4x2) {
  I32Type i32;
  MatrixType m{&i32, 4, 2};
  EXPECT_EQ(32u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(32u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, MinBufferBindingSize3x2) {
  I32Type i32;
  MatrixType m{&i32, 3, 2};
  EXPECT_EQ(28u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(28u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, MinBufferBindingSize2x3) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  EXPECT_EQ(24u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(24u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, MinBufferBindingSize2x2) {
  I32Type i32;
  MatrixType m{&i32, 2, 2};
  EXPECT_EQ(16u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, BaseAlignment4x2) {
  I32Type i32;
  MatrixType m{&i32, 4, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, BaseAlignment3x2) {
  I32Type i32;
  MatrixType m{&i32, 3, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, BaseAlignment2x3) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTypeTest, BaseAlignment2x2) {
  I32Type i32;
  MatrixType m{&i32, 2, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
