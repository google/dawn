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

#include "src/tint/type/multisampled_texture.h"

#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {
namespace {

using MultisampledTextureTest = TestHelper;

TEST_F(MultisampledTextureTest, Creation) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(TextureDimension::k2d, create<I32>());
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(MultisampledTextureTest, Hash) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(MultisampledTextureTest, Equals) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(TextureDimension::k2d, create<I32>());
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(MultisampledTextureTest, IsTexture) {
    F32 f32;
    MultisampledTexture s(TextureDimension::kCube, &f32);
    Texture* ty = &s;
    EXPECT_FALSE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<ExternalTexture>());
    EXPECT_TRUE(ty->Is<MultisampledTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(MultisampledTextureTest, Dim) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.dim(), TextureDimension::k3d);
}

TEST_F(MultisampledTextureTest, Type) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.type(), &f32);
}

TEST_F(MultisampledTextureTest, FriendlyName) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.FriendlyName(), "texture_multisampled_3d<f32>");
}

TEST_F(MultisampledTextureTest, Clone) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->dim(), TextureDimension::k2d);
    EXPECT_TRUE(mt->type()->Is<F32>());
}

}  // namespace
}  // namespace tint::type
