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

#include <string>
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/common/validation_mode.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/utils/text/string.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ExpressionList = Vector<const ast::Expression*, 8>;

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

TEST_F(ResolverCompatibilityModeTest, LinearInterpolation_Parameter) {
    // @fragment
    // fn main(@location(1) @interpolate(linear) value : f32) {
    // }

    Func("main",
         Vector{Param("value", ty.f32(),
                      Vector{
                          Location(1_i),
                          Interpolate(Source{{12, 34}}, core::InterpolationType::kLinear,
                                      core::InterpolationSampling::kCenter),
                      })},
         ty.void_(), Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: use of '@interpolate(linear)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, LinearInterpolation_StructMember) {
    // struct S {
    //   @location(1) @interpolate(linear) value : f32,
    // }

    Structure("S", Vector{
                       Member("value", ty.f32(),
                              Vector{
                                  Location(1_i),
                                  Interpolate(Source{{12, 34}}, core::InterpolationType::kLinear,
                                              core::InterpolationSampling::kCenter),
                              }),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: use of '@interpolate(linear)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleInterpolation_Parameter) {
    // @fragment
    // fn main(@location(1) @interpolate(perspective, sample) value : f32) {
    // }

    Func("main",
         Vector{Param("value", ty.f32(),
                      Vector{
                          Location(1_i),
                          Interpolate(Source{{12, 34}}, core::InterpolationType::kPerspective,
                                      core::InterpolationSampling::kSample),
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
        R"(12:34 error: use of '@interpolate(..., sample)' is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest, SampleInterpolation_StructMember) {
    // struct S {
    //   @location(1) @interpolate(perspective, sample) value : f32,
    // }

    Structure("S",
              Vector{
                  Member("value", ty.f32(),
                         Vector{
                             Location(1_i),
                             Interpolate(Source{{12, 34}}, core::InterpolationType::kPerspective,
                                         core::InterpolationSampling::kSample),
                         }),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@interpolate(..., sample)' is not allowed in compatibility mode)");
}

class ResolverCompatibilityModeTest_TextureLoad : public ResolverCompatibilityModeTest {
  protected:
    void add_call_param(std::string name, ast::Type type, ExpressionList* call_params) {
        const std::string type_name = type->identifier->symbol.Name();
        if (tint::HasPrefix(type_name, "texture")) {
            GlobalVar(name, type, Binding(0_a), Group(0_a));
        } else {
            GlobalVar(name, type, core::AddressSpace::kPrivate);
        }
        call_params->Push(Expr(Source{{12, 34}}, name));
    }
};

TEST_F(ResolverCompatibilityModeTest_TextureLoad, TextureDepth2D) {
    // textureLoad(someDepthTexture2D, coords, level)
    const ast::Type coords_type = ty.vec2(ty.i32());
    auto texture_type = ty.depth_texture(core::type::TextureDimension::k2d);

    ExpressionList call_params;

    add_call_param("texture", texture_type, &call_params);
    add_call_param("coords", coords_type, &call_params);
    add_call_param("level", ty.i32(), &call_params);

    auto* expr = Call("textureLoad", call_params);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of texture_depth_2d with textureLoad is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest_TextureLoad, TextureDepth2DArray) {
    // textureLoad(someDepthTexture2DArray, coords, layer, level)
    const ast::Type coords_type = ty.vec2(ty.i32());
    auto texture_type = ty.depth_texture(core::type::TextureDimension::k2dArray);

    ExpressionList call_params;

    add_call_param("texture", texture_type, &call_params);
    add_call_param("coords", coords_type, &call_params);
    add_call_param("array_index", ty.i32(), &call_params);
    add_call_param("level", ty.i32(), &call_params);

    auto* expr = Call("textureLoad", call_params);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of texture_depth_2d_array with textureLoad is not allowed in compatibility mode)");
}

TEST_F(ResolverCompatibilityModeTest_TextureLoad, TextureDepthMultisampled2D) {
    // textureLoad(someDepthTextureMultisampled2D, coords, sample_index)
    const ast::Type coords_type = ty.vec2(ty.i32());
    auto texture_type = ty.depth_multisampled_texture(core::type::TextureDimension::k2d);

    ExpressionList call_params;

    add_call_param("texture", texture_type, &call_params);
    add_call_param("coords", coords_type, &call_params);
    add_call_param("sample_index", ty.i32(), &call_params);

    auto* expr = Call("textureLoad", call_params);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of texture_depth_multisampled_2d with textureLoad is not allowed in compatibility mode)");
}

}  // namespace
}  // namespace tint::resolver
