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

#include "src/tint/type/texture.h"

#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/test_helper.h"

namespace tint::type {
namespace {

using TextureTypeDimTest = TestParamHelper<TextureDimension>;

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
                         ::testing::Values(TextureDimension::k1d,
                                           TextureDimension::k2d,
                                           TextureDimension::k2dArray,
                                           TextureDimension::k3d,
                                           TextureDimension::kCube,
                                           TextureDimension::kCubeArray));

}  // namespace
}  // namespace tint::type
