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

#include "src/type/storage_texture_type.h"

#include <memory>

#include "src/ast/identifier_expression.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/test_helper.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type_determiner.h"

namespace tint {
namespace type {
namespace {

using StorageTextureTest = TestHelper;

TEST_F(StorageTextureTest, Is) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Float);
  Type* ty = &s;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_TRUE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(StorageTextureTest, IsTexture) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Float);
  Texture* ty = &s;
  EXPECT_FALSE(ty->Is<DepthTexture>());
  EXPECT_FALSE(ty->Is<SampledTexture>());
  EXPECT_TRUE(ty->Is<StorageTexture>());
}

TEST_F(StorageTextureTest, Dim) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Float);
  EXPECT_EQ(s.dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Float);
  EXPECT_EQ(s.image_format(), ImageFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, TypeName) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Float);
  EXPECT_EQ(s.type_name(), "__storage_texture_2d_array_rgba32float");
}

TEST_F(StorageTextureTest, F32) {
  Type* s = create<StorageTexture>(TextureDimension::k2dArray,
                                   ImageFormat::kRgba32Float);
  TypeDeterminer td(this);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
  Type* s = create<StorageTexture>(TextureDimension::k2dArray,
                                   ImageFormat::kRg32Uint);
  TypeDeterminer td(this);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
  Type* s = create<StorageTexture>(TextureDimension::k2dArray,
                                   ImageFormat::kRgba32Sint);
  TypeDeterminer td(this);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<I32>());
}

TEST_F(StorageTextureTest, MinBufferBindingSize) {
  StorageTexture s(TextureDimension::k2dArray, ImageFormat::kRgba32Sint);
  EXPECT_EQ(0u, s.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace tint
