// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

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
        Enable(wgsl::Extension::kChromiumInternalDualSourceBlending);
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

class DualSourceBlendingExtensionTestWithParams : public ResolverTestWithParam<int> {
  public:
    DualSourceBlendingExtensionTestWithParams() {
        Enable(wgsl::Extension::kChromiumInternalDualSourceBlending);
    }
};

// Rendering to multiple render targets while using dual source blending should fail.
TEST_P(DualSourceBlendingExtensionTestWithParams, MultipleRenderTargetsNotAllowed) {
    Structure("Output",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), Index(0_a)}),
                  Member("b", ty.vec4<f32>(), Vector{Location(0_a), Index(1_a)}),
                  Member("c", ty.vec4<f32>(), Vector{Location(Source{{12, 34}}, AInt(GetParam()))}),
              });

    EXPECT_FALSE(r()->Resolve());
    StringStream err;
    err << "12:34 error: Multiple render targets are not allowed when using dual source blending. "
           "The output @location("
        << GetParam() << ") is not allowed as a render target.";
    EXPECT_EQ(r()->error(), err.str());
}

INSTANTIATE_TEST_SUITE_P(DualSourceBlendingExtensionTests,
                         DualSourceBlendingExtensionTestWithParams,
                         testing::Values(1, 2, 3, 4, 5, 6, 7));

}  // namespace
}  // namespace tint::resolver
