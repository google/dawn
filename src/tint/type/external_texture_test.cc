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

#include "src/tint/type/external_texture.h"

#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {
namespace {

using ExternalTextureTest = TestHelper;

TEST_F(ExternalTextureTest, Creation) {
    auto* a = create<ExternalTexture>();
    auto* b = create<ExternalTexture>();
    EXPECT_EQ(a, b);
}

TEST_F(ExternalTextureTest, Hash) {
    auto* a = create<ExternalTexture>();
    auto* b = create<ExternalTexture>();
    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ExternalTextureTest, Equals) {
    auto* a = create<ExternalTexture>();
    auto* b = create<ExternalTexture>();
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(Void{}));
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

TEST_F(ExternalTextureTest, FriendlyName) {
    ExternalTexture s;
    EXPECT_EQ(s.FriendlyName(), "texture_external");
}

TEST_F(ExternalTextureTest, Clone) {
    auto* a = create<ExternalTexture>();

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* b = a->Clone(ctx);
    ASSERT_TRUE(b->Is<ExternalTexture>());
}

}  // namespace
}  // namespace tint::type
