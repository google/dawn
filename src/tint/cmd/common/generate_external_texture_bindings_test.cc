// Copyright 2022 The Dawn & Tint Authors
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

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/api/common/binding_point.h"
#include "src/tint/cmd/common/generate_external_texture_bindings.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::cmd {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

constexpr auto kUniform = core::AddressSpace::kUniform;

class GenerateExternalTextureBindingsTest : public ::testing::Test {};

TEST_F(GenerateExternalTextureBindingsTest, None) {
    ProgramBuilder b;

    tint::Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(program);
    ASSERT_TRUE(bindings.empty());
}

TEST_F(GenerateExternalTextureBindingsTest, One) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));

    tint::Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(program);
    ASSERT_EQ(bindings.size(), 1u);

    auto to = bindings[BindingPoint{0, 0}];
    ASSERT_EQ(to.plane_1.group, 0u);
    ASSERT_EQ(to.params.group, 0u);
    ASSERT_EQ(to.plane_1.binding, 1u);
    ASSERT_EQ(to.params.binding, 2u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_SameGroup) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("v1", b.ty.external_texture(), b.Group(0_a), b.Binding(1_a));

    tint::Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 2u);
    ASSERT_EQ(to0.params.binding, 3u);

    auto to1 = bindings[BindingPoint{0, 1}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 4u);
    ASSERT_EQ(to1.params.binding, 5u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_DifferentGroup) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("v1", b.ty.external_texture(), b.Group(1_a), b.Binding(0_a));

    tint::Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 1u);
    ASSERT_EQ(to0.params.binding, 2u);

    auto to1 = bindings[BindingPoint{1, 0}];
    ASSERT_EQ(to1.plane_1.group, 1u);
    ASSERT_EQ(to1.params.group, 1u);
    ASSERT_EQ(to1.plane_1.binding, 1u);
    ASSERT_EQ(to1.params.binding, 2u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_WithOtherBindingsInSameGroup) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.i32(), b.Group(0_a), b.Binding(0_a), kUniform);
    b.GlobalVar("v1", b.ty.external_texture(), b.Group(0_a), b.Binding(1_a));
    b.GlobalVar("v2", b.ty.i32(), b.Group(0_a), b.Binding(2_a), kUniform);
    b.GlobalVar("v3", b.ty.external_texture(), b.Group(0_a), b.Binding(3_a));
    b.GlobalVar("v4", b.ty.i32(), b.Group(0_a), b.Binding(4_a), kUniform);

    tint::Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    auto bindings = GenerateExternalTextureBindings(program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[BindingPoint{0, 1}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 5u);
    ASSERT_EQ(to0.params.binding, 6u);

    auto to1 = bindings[BindingPoint{0, 3}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 7u);
    ASSERT_EQ(to1.params.binding, 8u);
}

}  // namespace
}  // namespace tint::cmd
