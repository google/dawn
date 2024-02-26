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

// Using the @blend_src attribute without chromium_internal_dual_source_blending enabled should
// fail.
TEST_F(DualSourceBlendingExtensionTest, UseBlendSrcAttribWithoutExtensionError) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@blend_src' requires enabling extension 'chromium_internal_dual_source_blending')");
}

class DualSourceBlendingExtensionTests : public ResolverTest {
  public:
    DualSourceBlendingExtensionTests() {
        Enable(wgsl::Extension::kChromiumInternalDualSourceBlending);
    }
};

// Using an F32 as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcF32Error) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_f)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_srci32' or 'u32' value");
}

// Using a floating point number as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcFloatValueError) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 1.0_a)}),
                        });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_srci32' or 'u32' value");
}

// Using a number less than zero as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcNegativeValue) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, -1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be zero or one");
}

// Using a number greater than one as an index value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcValueAboveOne) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 2_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be zero or one");
}

// Using an index value at the same location multiple times should fail.
TEST_F(DualSourceBlendingExtensionTests, DuplicateBlendSrces) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                            Member(Source{{12, 34}}, "b", ty.vec4<f32>(),
                                   Vector{Location(Source{{12, 34}}, 0_a), BlendSrc(0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@location(0) @blend_src(0)' appears multiple times");
}

// Using the index attribute without a location attribute should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithMissingLocationAttribute_Struct) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{BlendSrc(Source{{12, 34}}, 1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

// Using the index attribute without a location attribute should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithMissingLocationAttribute_ReturnValue) {
    Func("F", Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{
             BlendSrc(Source{{12, 34}}, 1_a),
             Builtin(core::BuiltinValue::kPointSize),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

// Using an index attribute on a struct member should pass.
TEST_F(DualSourceBlendingExtensionTests, StructMemberBlendSrcAttribute) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using an index attribute on a global variable should pass. This is needed internally when using
// @blend_src with the canonicalize_entry_point transform. This test uses an internal attribute to
// ignore address space, which is how it is used with the canonicalize_entry_point transform.
TEST_F(DualSourceBlendingExtensionTests, GlobalVariableBlendSrcAttribute) {
    GlobalVar(
        "var", ty.vec4<f32>(),
        Vector{Location(0_a), BlendSrc(0_a), Disable(ast::DisabledValidation::kIgnoreAddressSpace)},
        core::AddressSpace::kOut);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using the index attribute with a non-zero location should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithNonZeroLocation_Struct) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(1_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

// Using the index attribute with a non-zero location should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithNonZeroLocation_ReturnValue) {
    Func("F", Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{
             Location(1_a),
             BlendSrc(Source{{12, 34}}, 1_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

TEST_F(DualSourceBlendingExtensionTests, NoNonZeroCollisionsBetweenInAndOut) {
    // struct NonZeroLocation {
    //   @location(1) a : vec4<f32>,
    // };
    // struct NonZeroBlendSrc {
    //   @location(0) @blend_src(1) a : vec4<f32>,
    // };
    // fn X(in : NonZeroLocation) -> NonZeroBlendSrc { return NonZeroBlendSrc(); }
    // fn Y(in : NonZeroBlendSrc) -> NonZeroLocation { return NonZeroLocation(); }
    Structure("NonZeroLocation", Vector{
                                     Member("a", ty.vec4<f32>(), Vector{Location(1_a)}),
                                 });
    Structure("NonZeroBlendSrc",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
              });
    Func("X", Vector{Param("in", ty("NonZeroLocation"))}, ty("NonZeroBlendSrc"),
         Vector{Return(Call("NonZeroBlendSrc"))}, Vector{Stage(ast::PipelineStage::kFragment)});
    Func("Y", Vector{Param("in", ty("NonZeroBlendSrc"))}, ty("NonZeroLocation"),
         Vector{Return(Call("NonZeroLocation"))}, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(DualSourceBlendingExtensionTests, MixedBlendSrcAndNonBlendSrcOnLocationZero_Struct) {
    // struct S {
    //   @location(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<f32>,
    // };
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(),
                              Vector{
                                  Location(Source{{12, 34}}, 0_a),
                              }),
                       Member("b", ty.vec4<f32>(),
                              Vector{Location(0_a), BlendSrc(Source{{56, 78}}, 1_a)}),
                   });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(DualSourceBlendingExtensionTests,
       MixedBlendSrcAndNonBlendSrcOnLocationZero_StructUsedInEntryPoint) {
    // struct S {
    //   @location(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<f32>,
    // };
    // fn F() -> S { return S(); }
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(),
                              Vector{
                                  Location(Source{{12, 34}}, 0_a),
                              }),
                       Member("b", ty.vec4<f32>(),
                              Vector{Location(0_a), BlendSrc(Source{{56, 78}}, 1_a)}),
                   });
    Func("F", Empty, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@blend_src' requires all the output '@location' attributes of the entry point to be paired with a '@blend_src' attribute
56:78 note: use of '@blend_src' here
note: while analyzing entry point 'F')");
}

class DualSourceBlendingExtensionTestWithParams : public ResolverTestWithParam<int> {
  public:
    DualSourceBlendingExtensionTestWithParams() {
        Enable(wgsl::Extension::kChromiumInternalDualSourceBlending);
    }
};

// Rendering to multiple render targets while using dual source blending should fail.
TEST_P(DualSourceBlendingExtensionTestWithParams,
       MultipleRenderTargetsNotAllowed_BlendSrcAndNoBlendSrc) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<f32>,
    //   @location(n)           c : vec4<f32>,
    // };
    // fn F() -> S { return S(); }
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{1, 2}}, 1_a)}),
                  Member("c", ty.vec4<f32>(), Vector{Location(Source{{3, 4}}, AInt(GetParam()))}),
              });
    Func("F", Empty, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: use of '@blend_src' requires all the output '@location' attributes of the entry point to be paired with a '@blend_src' attribute
1:2 note: use of '@blend_src' here
note: while analyzing entry point 'F')");
}

TEST_P(DualSourceBlendingExtensionTestWithParams,
       MultipleRenderTargetsNotAllowed_NonZeroLocationThenBlendSrc) {
    // struct S {
    //   @location(n)           a : vec4<f32>,
    //   @location(0) @blend_src(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    // fn F() -> S { return S(); }
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(Source{{1, 2}}, AInt(GetParam()))}),
                  Member("b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{3, 4}}, 1_a)}),
              });
    Func(Source{{5, 6}}, "F", Empty, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: use of '@blend_src' requires all the output '@location' attributes of the entry point to be paired with a '@blend_src' attribute
note: use of '@blend_src' here
5:6 note: while analyzing entry point 'F')");
}

INSTANTIATE_TEST_SUITE_P(DualSourceBlendingExtensionTests,
                         DualSourceBlendingExtensionTestWithParams,
                         testing::Values(1, 2, 3, 4, 5, 6, 7));

}  // namespace
}  // namespace tint::resolver
