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

#include "src/tint/sem/depth_texture.h"

#include "src/tint/sem/test_helper.h"

#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"

namespace tint::sem {
namespace {

using DepthTextureTest = TestHelper;

TEST_F(DepthTextureTest, Creation) {
  auto* a = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* c = create<DepthTexture>(ast::TextureDimension::k2dArray);

  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}

TEST_F(DepthTextureTest, Hash) {
  auto* a = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* c = create<DepthTexture>(ast::TextureDimension::k2dArray);

  EXPECT_EQ(a->Hash(), b->Hash());
  EXPECT_NE(a->Hash(), c->Hash());
}

TEST_F(DepthTextureTest, Equals) {
  auto* a = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthTexture>(ast::TextureDimension::k2d);
  auto* c = create<DepthTexture>(ast::TextureDimension::k2dArray);

  EXPECT_TRUE(a->Equals(*b));
  EXPECT_FALSE(a->Equals(*c));
  EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(DepthTextureTest, IsTexture) {
  DepthTexture d(ast::TextureDimension::kCube);
  Texture* ty = &d;
  EXPECT_TRUE(ty->Is<DepthTexture>());
  EXPECT_FALSE(ty->Is<ExternalTexture>());
  EXPECT_FALSE(ty->Is<SampledTexture>());
  EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(DepthTextureTest, Dim) {
  DepthTexture d(ast::TextureDimension::kCube);
  EXPECT_EQ(d.dim(), ast::TextureDimension::kCube);
}

TEST_F(DepthTextureTest, FriendlyName) {
  DepthTexture d(ast::TextureDimension::kCube);
  EXPECT_EQ(d.FriendlyName(Symbols()), "texture_depth_cube");
}

}  // namespace
}  // namespace tint::sem
