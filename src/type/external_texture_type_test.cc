// Copyright 2021 The Tint Authors.
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

#include "src/type/external_texture_type.h"

#include "src/type/access_control_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/test_helper.h"

namespace tint {
namespace type {
namespace {

using ExternalTextureTest = TestHelper;

TEST_F(ExternalTextureTest, Is) {
  F32 f32;
  ExternalTexture s;
  Type* ty = &s;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<ArrayType>());
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

TEST_F(ExternalTextureTest, IsTexture) {
  F32 f32;
  ExternalTexture s;
  Texture* ty = &s;
  EXPECT_FALSE(ty->Is<DepthTexture>());
  EXPECT_TRUE(ty->Is<ExternalTexture>());
  EXPECT_FALSE(ty->Is<MultisampledTexture>());
  EXPECT_FALSE(ty->Is<SampledTexture>());
  EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(ExternalTextureTest, Dim) {
  F32 f32;
  ExternalTexture s;
  EXPECT_EQ(s.dim(), TextureDimension::k2d);
}

TEST_F(ExternalTextureTest, TypeName) {
  F32 f32;
  ExternalTexture s;
  EXPECT_EQ(s.type_name(), "__external_texture");
}

TEST_F(ExternalTextureTest, FriendlyName) {
  ExternalTexture s;
  EXPECT_EQ(s.FriendlyName(Symbols()), "texture_external");
}

}  // namespace
}  // namespace type
}  // namespace tint
