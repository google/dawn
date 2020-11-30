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
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using MatrixTest = TestHelper;

TEST_F(MatrixTest, Creation) {
  I32 i32;
  Matrix m{&i32, 2, 4};
  EXPECT_EQ(m.type(), &i32);
  EXPECT_EQ(m.rows(), 2u);
  EXPECT_EQ(m.columns(), 4u);
}

TEST_F(MatrixTest, Is) {
  I32 i32;
  Matrix m{&i32, 2, 3};
  Type* ty = &m;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_TRUE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(MatrixTest, TypeName) {
  I32 i32;
  Matrix m{&i32, 2, 3};
  EXPECT_EQ(m.type_name(), "__mat_2_3__i32");
}

TEST_F(MatrixTest, MinBufferBindingSize4x2) {
  I32 i32;
  Matrix m{&i32, 4, 2};
  EXPECT_EQ(32u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(32u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, MinBufferBindingSize3x2) {
  I32 i32;
  Matrix m{&i32, 3, 2};
  EXPECT_EQ(28u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(28u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, MinBufferBindingSize2x3) {
  I32 i32;
  Matrix m{&i32, 2, 3};
  EXPECT_EQ(24u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(24u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, MinBufferBindingSize2x2) {
  I32 i32;
  Matrix m{&i32, 2, 2};
  EXPECT_EQ(16u, m.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.MinBufferBindingSize(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, BaseAlignment4x2) {
  I32 i32;
  Matrix m{&i32, 4, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, BaseAlignment3x2) {
  I32 i32;
  Matrix m{&i32, 3, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, BaseAlignment2x3) {
  I32 i32;
  Matrix m{&i32, 2, 3};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(MatrixTest, BaseAlignment2x2) {
  I32 i32;
  Matrix m{&i32, 2, 2};
  EXPECT_EQ(16u, m.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(8u, m.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
