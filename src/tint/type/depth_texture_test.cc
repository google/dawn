// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/depth_texture.h"

#include "src/tint/type/external_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {
namespace {

using DepthTextureTest = TestHelper;

TEST_F(DepthTextureTest, Creation) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);
    auto* c = create<DepthTexture>(TextureDimension::k2dArray);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(DepthTextureTest, Hash) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(DepthTextureTest, Equals) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);
    auto* c = create<DepthTexture>(TextureDimension::k2dArray);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(DepthTextureTest, IsTexture) {
    DepthTexture d(TextureDimension::kCube);
    Texture* ty = &d;
    EXPECT_TRUE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<ExternalTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(DepthTextureTest, Dim) {
    DepthTexture d(TextureDimension::kCube);
    EXPECT_EQ(d.dim(), TextureDimension::kCube);
}

TEST_F(DepthTextureTest, FriendlyName) {
    DepthTexture d(TextureDimension::kCube);
    EXPECT_EQ(d.FriendlyName(), "texture_depth_cube");
}

TEST_F(DepthTextureTest, Clone) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* dt = a->Clone(ctx);
    EXPECT_EQ(dt->dim(), TextureDimension::k2d);
}

}  // namespace
}  // namespace tint::type
