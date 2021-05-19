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

#include "src/sem/multisampled_texture_type.h"

#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/test_helper.h"

namespace tint {
namespace sem {
namespace {

using MultisampledTextureTest = TestHelper;

TEST_F(MultisampledTextureTest, IsTexture) {
  F32 f32;
  MultisampledTexture s(ast::TextureDimension::kCube, &f32);
  Texture* ty = &s;
  EXPECT_FALSE(ty->Is<DepthTexture>());
  EXPECT_FALSE(ty->Is<ExternalTexture>());
  EXPECT_TRUE(ty->Is<MultisampledTexture>());
  EXPECT_FALSE(ty->Is<SampledTexture>());
  EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(MultisampledTextureTest, Dim) {
  F32 f32;
  MultisampledTexture s(ast::TextureDimension::k3d, &f32);
  EXPECT_EQ(s.dim(), ast::TextureDimension::k3d);
}

TEST_F(MultisampledTextureTest, Type) {
  F32 f32;
  MultisampledTexture s(ast::TextureDimension::k3d, &f32);
  EXPECT_EQ(s.type(), &f32);
}

TEST_F(MultisampledTextureTest, TypeName) {
  F32 f32;
  MultisampledTexture s(ast::TextureDimension::k3d, &f32);
  EXPECT_EQ(s.type_name(), "__multisampled_texture_3d__f32");
}

TEST_F(MultisampledTextureTest, FriendlyName) {
  F32 f32;
  MultisampledTexture s(ast::TextureDimension::k3d, &f32);
  EXPECT_EQ(s.FriendlyName(Symbols()), "texture_multisampled_3d<f32>");
}

}  // namespace
}  // namespace sem
}  // namespace tint
