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

#include "src/tint/sem/multisampled_texture.h"

#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/test_helper.h"

namespace tint::sem {
namespace {

using MultisampledTextureTest = TestHelper;

TEST_F(MultisampledTextureTest, Creation) {
    auto* a = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(ast::TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(ast::TextureDimension::k2d, create<I32>());
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(MultisampledTextureTest, Hash) {
    auto* a = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(ast::TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(ast::TextureDimension::k2d, create<I32>());
    EXPECT_EQ(a->Hash(), b->Hash());
    EXPECT_NE(a->Hash(), c->Hash());
    EXPECT_NE(a->Hash(), d->Hash());
}

TEST_F(MultisampledTextureTest, Equals) {
    auto* a = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(ast::TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(ast::TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(ast::TextureDimension::k2d, create<I32>());
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

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

TEST_F(MultisampledTextureTest, FriendlyName) {
    F32 f32;
    MultisampledTexture s(ast::TextureDimension::k3d, &f32);
    EXPECT_EQ(s.FriendlyName(Symbols()), "texture_multisampled_3d<f32>");
}

}  // namespace
}  // namespace tint::sem
