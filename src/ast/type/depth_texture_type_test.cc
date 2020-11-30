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

#include "src/ast/type/depth_texture_type.h"

#include "src/ast/test_helper.h"

#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using DepthTextureTypeTest = TestHelper;

TEST_F(DepthTextureTypeTest, Is) {
  DepthTextureType d(TextureDimension::kCube);
  Type* ty = &d;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->Is<SamplerType>());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_TRUE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(DepthTextureTypeTest, IsTextureType) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_TRUE(d.IsDepth());
  EXPECT_FALSE(d.IsSampled());
  EXPECT_FALSE(d.IsStorage());
}

TEST_F(DepthTextureTypeTest, Dim) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_EQ(d.dim(), TextureDimension::kCube);
}

TEST_F(DepthTextureTypeTest, TypeName) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_EQ(d.type_name(), "__depth_texture_cube");
}

TEST_F(DepthTextureTypeTest, MinBufferBindingSize) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_EQ(0u, d.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
