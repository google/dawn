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

#include "src/tint/sem/depth_multisampled_texture.h"

#include "src/tint/sem/test_helper.h"

#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"

namespace tint::sem {
namespace {

using DepthMultisampledTextureTest = TestHelper;

TEST_F(DepthMultisampledTextureTest, Creation) {
  auto* a = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);

  EXPECT_EQ(a, b);
}

TEST_F(DepthMultisampledTextureTest, Hash) {
  auto* a = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);

  EXPECT_EQ(a->Hash(), b->Hash());
}

TEST_F(DepthMultisampledTextureTest, Equals) {
  auto* a = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);
  auto* b = create<DepthMultisampledTexture>(ast::TextureDimension::k2d);

  EXPECT_TRUE(a->Equals(*a));
  EXPECT_TRUE(a->Equals(*b));
  EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(DepthMultisampledTextureTest, Dim) {
  DepthMultisampledTexture d(ast::TextureDimension::k2d);
  EXPECT_EQ(d.dim(), ast::TextureDimension::k2d);
}

TEST_F(DepthMultisampledTextureTest, FriendlyName) {
  DepthMultisampledTexture d(ast::TextureDimension::k2d);
  EXPECT_EQ(d.FriendlyName(Symbols()), "texture_depth_multisampled_2d");
}

}  // namespace
}  // namespace tint::sem
