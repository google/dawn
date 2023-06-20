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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using DualSourceBlendingExtensionTest = ResolverTest;

// Using the @index attribute without chromium_internal_dual_source_blending enabled should fail.
TEST_F(DualSourceBlendingExtensionTest, UseIndexAttribWithoutExtensionError) {
    Structure("Output", utils::Vector{
                            Member("a", ty.vec4<f32>(),
                                   utils::Vector{Location(0_a), Index(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: use of '@index' attribute requires enabling extension "
              "'chromium_internal_dual_source_blending'");
}

TEST_F(DualSourceBlendingExtensionTest, IndexF32Error) {
    Enable(builtin::Extension::kChromiumInternalDualSourceBlending);

    Structure("Output", utils::Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   utils::Vector{Location(0_a), Index(Source{{12, 34}}, 0_f)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @location must be an i32 or u32 value");
}

TEST_F(DualSourceBlendingExtensionTest, IndexFloatValueError) {
    Enable(builtin::Extension::kChromiumInternalDualSourceBlending);

    Structure("Output", utils::Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   utils::Vector{Location(0_a), Index(Source{{12, 34}}, 1.0_a)}),
                        });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @location must be an i32 or u32 value");
}

TEST_F(DualSourceBlendingExtensionTest, IndexNegativeValue) {
    Enable(builtin::Extension::kChromiumInternalDualSourceBlending);

    Structure("Output", utils::Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   utils::Vector{Location(0_a), Index(Source{{12, 34}}, -1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @index value must be zero or one");
}

TEST_F(DualSourceBlendingExtensionTest, IndexValueAboveOne) {
    Enable(builtin::Extension::kChromiumInternalDualSourceBlending);

    Structure("Output", utils::Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   utils::Vector{Location(0_a), Index(Source{{12, 34}}, 2_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @index value must be zero or one");
}

}  // namespace
}  // namespace tint::resolver
