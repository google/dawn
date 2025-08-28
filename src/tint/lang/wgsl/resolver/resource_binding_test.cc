// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/resource_binding.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverResourceBindingTest = ResolverTest;

TEST_F(ResolverResourceBindingTest, ValidGlobalDecl) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    auto* var = GlobalVar("a", Binding(0_a), Group(0_a), ty("resource_binding"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(var)->UnwrapRef()->As<core::type::ResourceBinding>();
    ASSERT_NE(m, nullptr);
}

TEST_F(ResolverResourceBindingTest, InvalidNoFeature) {
    GlobalVar("a", Binding(0_a), Group(0_a), ty("resource_binding"));

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(
        resolver.error(),
        R"(error: use of a 'resource_binding' requires enabling extension 'chromium_experimental_dynamic_binding')");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclNoGroup) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", Binding(0_a), ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclNoBinding) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", Group(0_a), ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclPrivate) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", private_, ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: variables of type 'resource_binding' must not specify an address space)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclWorkgroup) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", workgroup, ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: variables of type 'resource_binding' must not specify an address space)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclStorageWithHandleType) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", storage, Binding(0_a), Group(0_a), ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: type 'resource_binding' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating 'var' a)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclUniformWithHandleType) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    GlobalVar("a", uniform, Binding(0_a), Group(0_a), ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: type 'resource_binding' cannot be used in address space 'uniform' as it is non-host-shareable
note: while instantiating 'var' a)");
}

TEST_F(ResolverResourceBindingTest, InvalidGlobalDeclOverride) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    Override("a", ty("resource_binding"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource_binding cannot be used as the type of a 'override')");
}

TEST_F(ResolverResourceBindingTest, InvalidFuncDecl) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty("resource_binding"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: function-scope 'var' must have a constructible type)");
}

TEST_F(ResolverResourceBindingTest, InvalidStructMember) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    Structure("S", Vector{Member("a", ty("resource_binding"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource_binding cannot be used as the type of a structure member)");
}

TEST_F(ResolverResourceBindingTest, InvalidAsFunctionParameter) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    Func("foo", Vector{Param("a", ty("resource_binding"))}, ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: type of function parameter cannot be resource_binding)");
}

TEST_F(ResolverResourceBindingTest, InvalidFunctionPointerParameterWithHandleType) {
    Enable(wgsl::Extension::kChromiumExperimentalDynamicBinding);
    Func("foo", Vector{Param("a", ty.ptr<function>(ty("resource_binding")))}, ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: pointer can not be formed to handle type resource_binding)");
}

}  // namespace
}  // namespace tint::resolver
