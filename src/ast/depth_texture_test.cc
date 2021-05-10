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

#include "src/ast/depth_texture.h"
#include "src/ast/access_control.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampled_texture.h"
#include "src/ast/sampler.h"
#include "src/ast/storage_texture.h"
#include "src/ast/struct.h"
#include "src/ast/test_helper.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
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
  EXPECT_EQ(d->dim(), TextureDimension::kCube);
}

TEST_F(AstDepthTextureTest, TypeName) {
  auto* d = create<DepthTexture>(TextureDimension::kCube);
  EXPECT_EQ(d->type_name(), "__depth_texture_cube");
}

TEST_F(AstDepthTextureTest, FriendlyName) {
  auto* d = create<DepthTexture>(TextureDimension::kCube);
  EXPECT_EQ(d->FriendlyName(Symbols()), "texture_depth_cube");
}

}  // namespace
}  // namespace ast
}  // namespace tint
