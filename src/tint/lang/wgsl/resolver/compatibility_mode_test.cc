// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/common/validation_mode.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class ResolverCompatibilityModeTest : public ResolverTest {
  protected:
    ResolverCompatibilityModeTest() {
        resolver_ = std::make_unique<Resolver>(this, wgsl::AllowedFeatures::Everything(),
                                               wgsl::ValidationMode::kCompat);
    }
};

TEST_F(ResolverCompatibilityModeTest, SampleMask_Parameter) {
    // @fragment
    // fn main(@builtin(sample_mask) mask : u32) {
    // }

    Func("main",
         Vector{Param("mask", ty.i32(),
                      Vector{
                          create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_mask")),
                      })},
         ty.void_(), Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_mask)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleMask_ReturnValue) {
    // @fragment
    // fn main() -> @builtin(sample_mask) u32 {
    //   return 0;
    // }

    Func("main", Empty, ty.u32(),
         Vector{
             Return(0_u),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_mask")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_mask)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleMask_StructMember) {
    // struct S {
    //   @builtin(sample_mask) mask : u32,
    // }

    Structure(
        "S",
        Vector{
            Member("mask", ty.u32(),
                   Vector{
                       create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_mask")),
                   }),
        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_mask)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleIndex_Parameter) {
    // @fragment
    // fn main(@builtin(sample_index) mask : u32) {
    // }

    Func("main",
         Vector{Param("mask", ty.i32(),
                      Vector{
                          create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_index")),
                      })},
         ty.void_(), Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_index)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleIndex_ReturnValue) {
    // @fragment
    // fn main() -> @builtin(sample_index) u32 {
    //   return 0;
    // }

    Func("main", Empty, ty.u32(),
         Vector{
             Return(0_u),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_index")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_index)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleIndex_StructMember) {
    // struct S {
    //   @builtin(sample_index) mask : u32,
    // }

    Structure(
        "S",
        Vector{
            Member("mask", ty.u32(),
                   Vector{
                       create<ast::BuiltinAttribute>({}, Expr(Source{{12, 34}}, "sample_index")),
                   }),
        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@builtin(sample_index)' is not allowed in compatibility mode)");
}

}  // namespace
}  // namespace tint::resolver
