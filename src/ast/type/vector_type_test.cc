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

#include "src/ast/type/vector_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using VectorTypeTest = TestHelper;

TEST_F(VectorTypeTest, Creation) {
  I32Type i32;
  VectorType v{&i32, 2};
  EXPECT_EQ(v.type(), &i32);
  EXPECT_EQ(v.size(), 2u);
}

TEST_F(VectorTypeTest, Is) {
  I32Type i32;
  VectorType v{&i32, 4};
  Type* ty = &v;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->Is<SamplerType>());
  EXPECT_FALSE(ty->Is<StructType>());
  EXPECT_FALSE(ty->Is<TextureType>());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_TRUE(ty->IsVector());
}

TEST_F(VectorTypeTest, TypeName) {
  I32Type i32;
  VectorType v{&i32, 3};
  EXPECT_EQ(v.type_name(), "__vec_3__i32");
}

TEST_F(VectorTypeTest, MinBufferBindingSizeVec2) {
  I32Type i32;
  VectorType v{&i32, 2};
  EXPECT_EQ(8u, v.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(VectorTypeTest, MinBufferBindingSizeVec3) {
  I32Type i32;
  VectorType v{&i32, 3};
  EXPECT_EQ(12u, v.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(VectorTypeTest, MinBufferBindingSizeVec4) {
  I32Type i32;
  VectorType v{&i32, 4};
  EXPECT_EQ(16u, v.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(VectorTypeTest, BaseAlignmentVec2) {
  I32Type i32;
  VectorType v{&i32, 2};
  EXPECT_EQ(8u, v.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(VectorTypeTest, BaseAlignmentVec3) {
  I32Type i32;
  VectorType v{&i32, 3};
  EXPECT_EQ(16u, v.BaseAlignment(MemoryLayout::kUniformBuffer));
}

TEST_F(VectorTypeTest, BaseAlignmentVec4) {
  I32Type i32;
  VectorType v{&i32, 4};
  EXPECT_EQ(16u, v.BaseAlignment(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
