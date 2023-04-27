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

#include "gmock/gmock.h"

#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverUnresolvedIdentifierSuggestions = ResolverTest;

TEST_F(ResolverUnresolvedIdentifierSuggestions, AddressSpace) {
    AST().AddGlobalVariable(create<ast::Var>(
        Ident("v"),                        // name
        ty.i32(),                          // type
        Expr(Source{{12, 34}}, "privte"),  // declared_address_space
        nullptr,                           // declared_access
        nullptr,                           // initializer
        utils::Empty                       // attributes
        ));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved address space 'privte'
12:34 note: Did you mean 'private'?
Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, BuiltinValue) {
    Func("f",
         utils::Vector{
             Param("p", ty.i32(), utils::Vector{Builtin(Expr(Source{{12, 34}}, "positon"))})},
         ty.void_(), utils::Empty, utils::Vector{Stage(ast::PipelineStage::kVertex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved builtin value 'positon'
12:34 note: Did you mean 'position'?
Possible values: 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'vertex_index', 'workgroup_id')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, TexelFormat) {
    GlobalVar("v", ty("texture_storage_1d", Expr(Source{{12, 34}}, "rba8unorm"), "read"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved texel format 'rba8unorm'
12:34 note: Did you mean 'rgba8unorm'?
Possible values: 'bgra8unorm', 'r32float', 'r32sint', 'r32uint', 'rg32float', 'rg32sint', 'rg32uint', 'rgba16float', 'rgba16sint', 'rgba16uint', 'rgba32float', 'rgba32sint', 'rgba32uint', 'rgba8sint', 'rgba8snorm', 'rgba8uint', 'rgba8unorm')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, AccessMode) {
    AST().AddGlobalVariable(create<ast::Var>(Ident("v"),       // name
                                             ty.i32(),         // type
                                             Expr("private"),  // declared_address_space
                                             Expr(Source{{12, 34}}, "reed"),  // declared_access
                                             nullptr,                         // initializer
                                             utils::Empty                     // attributes
                                             ));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved access 'reed'
12:34 note: Did you mean 'read'?
Possible values: 'read', 'read_write', 'write')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, InterpolationSampling) {
    Structure("s", utils::Vector{
                       Member("m", ty.vec4<f32>(),
                              utils::Vector{
                                  Interpolate(builtin::InterpolationType::kLinear,
                                              Expr(Source{{12, 34}}, "centre")),
                              }),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved interpolation sampling 'centre'
12:34 note: Did you mean 'center'?
Possible values: 'center', 'centroid', 'sample')");
}

TEST_F(ResolverUnresolvedIdentifierSuggestions, InterpolationType) {
    Structure("s", utils::Vector{
                       Member("m", ty.vec4<f32>(),
                              utils::Vector{
                                  Interpolate(Expr(Source{{12, 34}}, "liner")),
                              }),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved interpolation type 'liner'
12:34 note: Did you mean 'linear'?
Possible values: 'flat', 'linear', 'perspective')");
}

}  // namespace
}  // namespace tint::resolver
