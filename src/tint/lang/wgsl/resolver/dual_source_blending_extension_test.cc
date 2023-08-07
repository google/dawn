// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using DualSourceBlendingExtensionTest = ResolverTest;

// Using the @index attribute without chromium_internal_dual_source_blending enabled should fail.
TEST_F(DualSourceBlendingExtensionTest, UseIndexAttribWithoutExtensionError) {
    Structure("Output",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), Index(Source{{12, 34}}, 0_a)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: use of '@index' attribute requires enabling extension "
              "'chromium_internal_dual_source_blending'");
}

class DualSourceBlendingExtensionTests : public ResolverTest {
  public:
    DualSourceBlendingExtensionTests() {
        Enable(core::Extension::kChromiumInternalDualSourceBlending);
    }
};

// Using an F32 as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexF32Error) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), Index(Source{{12, 34}}, 0_f)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @location must be an i32 or u32 value");
}

// Using a floating point number as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexFloatValueError) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), Index(Source{{12, 34}}, 1.0_a)}),
                        });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @location must be an i32 or u32 value");
}

// Using a number less than zero as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexNegativeValue) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), Index(Source{{12, 34}}, -1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @index value must be zero or one");
}

// Using a number greater than one as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexValueAboveOne) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), Index(Source{{12, 34}}, 2_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @index value must be zero or one");
}

// Using an index value at the same location multiple times should fail.
TEST_F(DualSourceBlendingExtensionTests, DuplicateIndexes) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(), Vector{Location(0_a), Index(0_a)}),
                            Member(Source{{12, 34}}, "b", ty.vec4<f32>(),
                                   Vector{Location(0_a), Index(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @location(0) @index(0) appears multiple times");
}

// Using the index attribute without a location attribute should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexWithMissingLocationAttribute) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Index(Source{{12, 34}}, 1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index attribute must only be used with @location");
}

// Using an index attribute on a struct member should pass.
TEST_F(DualSourceBlendingExtensionTests, StructMemberIndexAttribute) {
    Structure("Output",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), Index(Source{{12, 34}}, 0_a)}),
              });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using an index attribute on a global variable should pass. This is needed internally when using
// @index with the canonicalize_entry_point transform. This test uses an internal attribute to
// ignore address space, which is how it is used with the canonicalize_entry_point transform.
TEST_F(DualSourceBlendingExtensionTests, GlobalVariableIndexAttribute) {
    GlobalVar(
        "var", ty.vec4<f32>(),
        Vector{Location(0_a), Index(0_a), Disable(ast::DisabledValidation::kIgnoreAddressSpace)},
        core::AddressSpace::kOut);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using the index attribute with a non-zero location should fail.
TEST_F(DualSourceBlendingExtensionTests, IndexWithNonZeroLocation) {
    Structure("Output",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(1_a), Index(Source{{12, 34}}, 0_a)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index attribute must only be used with @location(0)");
}

}  // namespace
}  // namespace tint::resolver
