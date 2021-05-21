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

#include "src/sem/texture_type.h"

#include "src/sem/sampled_texture_type.h"
#include "src/sem/test_helper.h"

namespace tint {
namespace sem {
namespace {

using TextureTypeDimTest = TestParamHelper<ast::TextureDimension>;

TEST_P(TextureTypeDimTest, DimMustMatch) {
  // Check that the dim() query returns the right dimensionality.
  F32 f32;
  // TextureType is an abstract class, so use concrete class
  // SampledTexture in its stead.
  SampledTexture st(GetParam(), &f32);
  EXPECT_EQ(st.dim(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(Dimensions,
                         TextureTypeDimTest,
                         ::testing::Values(ast::TextureDimension::k1d,
                                           ast::TextureDimension::k2d,
                                           ast::TextureDimension::k2dArray,
                                           ast::TextureDimension::k3d,
                                           ast::TextureDimension::kCube,
                                           ast::TextureDimension::kCubeArray));

}  // namespace
}  // namespace sem
}  // namespace tint
