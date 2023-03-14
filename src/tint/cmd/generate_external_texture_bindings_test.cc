// Copyright 2022 The Tint Authors.
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

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/cmd/generate_external_texture_bindings.h"
#include "src/tint/program_builder.h"
#include "src/tint/writer/binding_point.h"

namespace tint::cmd {
namespace {

using namespace tint::number_suffixes;  // NOLINT

constexpr auto kUniform = builtin::AddressSpace::kUniform;

class GenerateExternalTextureBindingsTest : public ::testing::Test {};

TEST_F(GenerateExternalTextureBindingsTest, None) {
    ProgramBuilder b;

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_TRUE(bindings.empty());
}

TEST_F(GenerateExternalTextureBindingsTest, One) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 1u);

    auto to = bindings[writer::BindingPoint{0, 0}];
    ASSERT_EQ(to.plane_1.group, 0u);
    ASSERT_EQ(to.params.group, 0u);
    ASSERT_EQ(to.plane_1.binding, 1u);
    ASSERT_EQ(to.params.binding, 2u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_SameGroup) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("v1", b.ty.external_texture(), b.Group(0_a), b.Binding(1_a));

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[writer::BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 2u);
    ASSERT_EQ(to0.params.binding, 3u);

    auto to1 = bindings[writer::BindingPoint{0, 1}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 4u);
    ASSERT_EQ(to1.params.binding, 5u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_DifferentGroup) {
    ProgramBuilder b;
    b.GlobalVar("v0", b.ty.external_texture(), b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("v1", b.ty.external_texture(), b.Group(1_a), b.Binding(0_a));

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[writer::BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 1u);
    ASSERT_EQ(to0.params.binding, 2u);

    auto to1 = bindings[writer::BindingPoint{1, 0}];
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

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[writer::BindingPoint{0, 1}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 5u);
    ASSERT_EQ(to0.params.binding, 6u);

    auto to1 = bindings[writer::BindingPoint{0, 3}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 7u);
    ASSERT_EQ(to1.params.binding, 8u);
}

}  // namespace
}  // namespace tint::cmd
