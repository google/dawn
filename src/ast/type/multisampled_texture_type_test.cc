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

#include "src/ast/type/multisampled_texture_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using MultisampledTextureTypeTest = TestHelper;

TEST_F(MultisampledTextureTypeTest, Is) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::kCube, &f32);
  Type* ty = &s;
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
  EXPECT_TRUE(ty->Is<TextureType>());
  EXPECT_FALSE(ty->Is<U32Type>());
  EXPECT_FALSE(ty->Is<VectorType>());
}

TEST_F(MultisampledTextureTypeTest, IsTextureType) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::kCube, &f32);
  TextureType* ty = &s;
  EXPECT_FALSE(ty->Is<DepthTextureType>());
  EXPECT_TRUE(ty->Is<MultisampledTextureType>());
  EXPECT_FALSE(ty->IsSampled());
  EXPECT_FALSE(ty->Is<StorageTextureType>());
}

TEST_F(MultisampledTextureTypeTest, Dim) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::k3d, &f32);
  EXPECT_EQ(s.dim(), TextureDimension::k3d);
}

TEST_F(MultisampledTextureTypeTest, Type) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::k3d, &f32);
  EXPECT_EQ(s.type(), &f32);
}

TEST_F(MultisampledTextureTypeTest, TypeName) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::k3d, &f32);
  EXPECT_EQ(s.type_name(), "__multisampled_texture_3d__f32");
}

TEST_F(MultisampledTextureTypeTest, MinBufferBindingSize) {
  F32Type f32;
  MultisampledTextureType s(TextureDimension::k3d, &f32);
  EXPECT_EQ(0u, s.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
