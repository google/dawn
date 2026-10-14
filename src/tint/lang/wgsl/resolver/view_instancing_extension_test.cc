// Copyright 2026 The Dawn & Tint Authors
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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

using ResolverViewInstancingExtensionTest = ResolverTest;

TEST_F(ResolverViewInstancingExtensionTest, RequiresExtension) {
    Structure("Inputs",
              Vector{Member("view", ty.u32(), Vector{Builtin(core::BuiltinValue::kViewIndex)})});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(view_index)' requires enabling extension 'view_instancing')");
}

TEST_F(ResolverViewInstancingExtensionTest, VertexInput) {
    Enable(wgsl::Extension::kViewInstancing);
    Func("main", Vector{Param("view", ty.u32(), Vector{Builtin(core::BuiltinValue::kViewIndex)})},
         ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>())},
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverViewInstancingExtensionTest, FragmentInput) {
    Enable(wgsl::Extension::kViewInstancing);
    Func("main", Vector{Param("view", ty.u32(), Vector{Builtin(core::BuiltinValue::kViewIndex)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverViewInstancingExtensionTest, RequiresU32) {
    Enable(wgsl::Extension::kViewInstancing);
    Structure("Inputs",
              Vector{Member("view", ty.i32(), Vector{Builtin(core::BuiltinValue::kViewIndex)})});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: store type of '@builtin(view_index)' must be 'u32'");
}

TEST_F(ResolverViewInstancingExtensionTest, ComputeInputInvalid) {
    Enable(wgsl::Extension::kViewInstancing);
    Func("main", Vector{Param("view", ty.u32(), Vector{Builtin(core::BuiltinValue::kViewIndex)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: '@builtin(view_index)' cannot be used for compute shader input");
}

TEST_F(ResolverViewInstancingExtensionTest, FragmentOutputInvalid) {
    Enable(wgsl::Extension::kViewInstancing);
    Func("main", Empty, ty.u32(), Vector{Return(Call<u32>())},
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Builtin(core::BuiltinValue::kViewIndex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: '@builtin(view_index)' cannot be used for fragment shader output");
}

}  // namespace
}  // namespace tint::resolver
