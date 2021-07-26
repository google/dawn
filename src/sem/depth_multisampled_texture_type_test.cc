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

#include "src/sem/depth_multisampled_texture_type.h"

#include "src/sem/test_helper.h"

#include "src/sem/external_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"

namespace tint {
namespace sem {
namespace {

using DepthMultisampledTextureTest = TestHelper;

TEST_F(DepthMultisampledTextureTest, Dim) {
  DepthMultisampledTexture d(ast::TextureDimension::k2d);
  EXPECT_EQ(d.dim(), ast::TextureDimension::k2d);
}

TEST_F(DepthMultisampledTextureTest, TypeName) {
  DepthMultisampledTexture d(ast::TextureDimension::k2d);
  EXPECT_EQ(d.type_name(), "__depth_multisampled_texture_2d");
}

TEST_F(DepthMultisampledTextureTest, FriendlyName) {
  DepthMultisampledTexture d(ast::TextureDimension::k2d);
  EXPECT_EQ(d.FriendlyName(Symbols()), "texture_depth_multisampled_2d");
}

}  // namespace
}  // namespace sem
}  // namespace tint
