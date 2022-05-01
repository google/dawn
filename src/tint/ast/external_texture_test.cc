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

#include "src/tint/ast/external_texture.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstExternalTextureTest = TestHelper;

TEST_F(AstExternalTextureTest, IsTexture) {
    Texture* ty = create<ExternalTexture>();
    EXPECT_FALSE(ty->Is<DepthTexture>());
    EXPECT_TRUE(ty->Is<ExternalTexture>());
    EXPECT_FALSE(ty->Is<MultisampledTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(AstExternalTextureTest, Dim) {
    auto* ty = create<ExternalTexture>();
    EXPECT_EQ(ty->dim, ast::TextureDimension::k2d);
}

TEST_F(AstExternalTextureTest, FriendlyName) {
    auto* ty = create<ExternalTexture>();
    EXPECT_EQ(ty->FriendlyName(Symbols()), "texture_external");
}

}  // namespace
}  // namespace tint::ast
