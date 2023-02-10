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

#include "src/tint/ast/sampled_texture.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstSampledTextureTest = TestHelper;

TEST_F(AstSampledTextureTest, IsTexture) {
    Texture* t = create<SampledTexture>(type::TextureDimension::kCube, ty.f32());
    EXPECT_TRUE(t->Is<SampledTexture>());
}

TEST_F(AstSampledTextureTest, Dim) {
    auto* s = create<SampledTexture>(type::TextureDimension::k3d, ty.f32());
    EXPECT_EQ(s->dim, type::TextureDimension::k3d);
}

TEST_F(AstSampledTextureTest, Type) {
    auto* s = create<SampledTexture>(type::TextureDimension::k3d, ty.f32());
    ASSERT_TRUE(s->type->Is<ast::TypeName>());
    EXPECT_EQ(Symbols().NameFor(s->type->As<ast::TypeName>()->name->symbol), "f32");
}

TEST_F(AstSampledTextureTest, FriendlyName) {
    auto* s = create<SampledTexture>(type::TextureDimension::k3d, ty.f32());
    EXPECT_EQ(s->FriendlyName(Symbols()), "texture_3d<f32>");
}

}  // namespace
}  // namespace tint::ast
