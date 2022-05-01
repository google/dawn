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

#include "src/tint/ast/multisampled_texture.h"

#include "src/tint/ast/access.h"
#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/ast/texture.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/vector.h"

namespace tint::ast {
namespace {

using AstMultisampledTextureTest = TestHelper;

TEST_F(AstMultisampledTextureTest, IsTexture) {
    auto* f32 = create<F32>();
    Texture* ty = create<MultisampledTexture>(TextureDimension::kCube, f32);
    EXPECT_FALSE(ty->Is<DepthTexture>());
    EXPECT_TRUE(ty->Is<MultisampledTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(AstMultisampledTextureTest, Dim) {
    auto* f32 = create<F32>();
    auto* s = create<MultisampledTexture>(TextureDimension::k3d, f32);
    EXPECT_EQ(s->dim, TextureDimension::k3d);
}

TEST_F(AstMultisampledTextureTest, Type) {
    auto* f32 = create<F32>();
    auto* s = create<MultisampledTexture>(TextureDimension::k3d, f32);
    EXPECT_EQ(s->type, f32);
}

TEST_F(AstMultisampledTextureTest, FriendlyName) {
    auto* f32 = create<F32>();
    auto* s = create<MultisampledTexture>(TextureDimension::k3d, f32);
    EXPECT_EQ(s->FriendlyName(Symbols()), "texture_multisampled_3d<f32>");
}

}  // namespace
}  // namespace tint::ast
