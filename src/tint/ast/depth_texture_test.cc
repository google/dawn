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

#include "src/tint/ast/depth_texture.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstDepthTextureTest = TestHelper;

TEST_F(AstDepthTextureTest, IsTexture) {
    Texture* ty = create<DepthTexture>(TextureDimension::kCube);
    EXPECT_TRUE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(AstDepthTextureTest, Dim) {
    auto* d = create<DepthTexture>(TextureDimension::kCube);
    EXPECT_EQ(d->dim, TextureDimension::kCube);
}

TEST_F(AstDepthTextureTest, FriendlyName) {
    auto* d = create<DepthTexture>(TextureDimension::kCube);
    EXPECT_EQ(d->FriendlyName(Symbols()), "texture_depth_cube");
}

}  // namespace
}  // namespace tint::ast
