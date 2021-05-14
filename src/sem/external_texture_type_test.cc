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

#include "src/sem/external_texture_type.h"

#include "src/sem/depth_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/test_helper.h"

namespace tint {
namespace sem {
namespace {

using ExternalTextureTest = TestHelper;

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
  EXPECT_EQ(s.dim(), ast::TextureDimension::k2d);
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
}  // namespace sem
}  // namespace tint
