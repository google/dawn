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

#include "src/tint/type/depth_multisampled_texture.h"

#include "src/tint/type/external_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {
namespace {

using DepthMultisampledTextureTest = TestHelper;

TEST_F(DepthMultisampledTextureTest, Creation) {
    auto* a = create<DepthMultisampledTexture>(TextureDimension::k2d);
    auto* b = create<DepthMultisampledTexture>(TextureDimension::k2d);

    EXPECT_EQ(a, b);
}

TEST_F(DepthMultisampledTextureTest, Hash) {
    auto* a = create<DepthMultisampledTexture>(TextureDimension::k2d);
    auto* b = create<DepthMultisampledTexture>(TextureDimension::k2d);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(DepthMultisampledTextureTest, Equals) {
    auto* a = create<DepthMultisampledTexture>(TextureDimension::k2d);
    auto* b = create<DepthMultisampledTexture>(TextureDimension::k2d);

    EXPECT_TRUE(a->Equals(*a));
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(DepthMultisampledTextureTest, Dim) {
    DepthMultisampledTexture d(TextureDimension::k2d);
    EXPECT_EQ(d.dim(), TextureDimension::k2d);
}

TEST_F(DepthMultisampledTextureTest, FriendlyName) {
    DepthMultisampledTexture d(TextureDimension::k2d);
    EXPECT_EQ(d.FriendlyName(), "texture_depth_multisampled_2d");
}

TEST_F(DepthMultisampledTextureTest, Clone) {
    auto* a = create<DepthMultisampledTexture>(TextureDimension::k2d);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* dt = a->Clone(ctx);
    EXPECT_EQ(dt->dim(), TextureDimension::k2d);
}

}  // namespace
}  // namespace tint::type
