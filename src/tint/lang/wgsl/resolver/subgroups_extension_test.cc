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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverSubgroupsExtensionTest = ResolverTest;

// Using a subgroup_size builtin attribute without chromium_experimental_subgroups enabled should
// fail.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupSizeAttribWithoutExtensionError) {
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of @builtin(subgroup_size) attribute requires enabling extension 'chromium_experimental_subgroups')");
}

// Using a subgroup_invocation_id builtin attribute without chromium_experimental_subgroups enabled
// should fail.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupInvocationIdAttribWithoutExtensionError) {
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of @builtin(subgroup_invocation_id) attribute requires enabling extension 'chromium_experimental_subgroups')");
}

// Using an i32 for a subgroup_size builtin input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeI32Error) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: store type of @builtin(subgroup_size) must be 'u32'");
}

// Using an i32 for a subgroup_invocation_id builtin input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupInvocationIdI32Error) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: store type of @builtin(subgroup_invocation_id) must be 'u32'");
}

// Using builtin(subgroup_size) for anything other than a compute shader input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeFragmentShader) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
    Func("main",
         Vector{Param("size", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: @builtin(subgroup_size) is only valid as a compute shader input");
}

// Using builtin(subgroup_invocation_id) for anything other than a compute shader input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupInvocationIdFragmentShader) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
    Func("main",
         Vector{Param("id", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: @builtin(subgroup_invocation_id) is only valid as a compute shader input");
}

}  // namespace
}  // namespace tint::resolver
