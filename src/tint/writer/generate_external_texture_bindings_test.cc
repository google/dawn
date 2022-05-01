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
#include "src/tint/program_builder.h"
#include "src/tint/writer/generate_external_texture_bindings.h"

namespace tint::writer {
namespace {

constexpr auto kUniform = ast::StorageClass::kUniform;

class GenerateExternalTextureBindingsTest : public ::testing::Test {};

TEST_F(GenerateExternalTextureBindingsTest, None) {
    ProgramBuilder b;
    b.WrapInFunction();

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_TRUE(bindings.empty());
}

TEST_F(GenerateExternalTextureBindingsTest, One) {
    ProgramBuilder b;
    b.Global("v0", b.ty.external_texture(), b.GroupAndBinding(0, 0));
    b.WrapInFunction();

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 1u);

    auto to = bindings[transform::BindingPoint{0, 0}];
    ASSERT_EQ(to.plane_1.group, 0u);
    ASSERT_EQ(to.params.group, 0u);
    ASSERT_EQ(to.plane_1.binding, 1u);
    ASSERT_EQ(to.params.binding, 2u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_SameGroup) {
    ProgramBuilder b;
    b.Global("v0", b.ty.external_texture(), b.GroupAndBinding(0, 0));
    b.Global("v1", b.ty.external_texture(), b.GroupAndBinding(0, 1));
    b.WrapInFunction();

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[transform::BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 2u);
    ASSERT_EQ(to0.params.binding, 3u);

    auto to1 = bindings[transform::BindingPoint{0, 1}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 4u);
    ASSERT_EQ(to1.params.binding, 5u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_DifferentGroup) {
    ProgramBuilder b;
    b.Global("v0", b.ty.external_texture(), b.GroupAndBinding(0, 0));
    b.Global("v1", b.ty.external_texture(), b.GroupAndBinding(1, 0));
    b.WrapInFunction();

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid());
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[transform::BindingPoint{0, 0}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 1u);
    ASSERT_EQ(to0.params.binding, 2u);

    auto to1 = bindings[transform::BindingPoint{1, 0}];
    ASSERT_EQ(to1.plane_1.group, 1u);
    ASSERT_EQ(to1.params.group, 1u);
    ASSERT_EQ(to1.plane_1.binding, 1u);
    ASSERT_EQ(to1.params.binding, 2u);
}

TEST_F(GenerateExternalTextureBindingsTest, Two_WithOtherBindingsInSameGroup) {
    ProgramBuilder b;
    b.Global("v0", b.ty.i32(), b.GroupAndBinding(0, 0), kUniform);
    b.Global("v1", b.ty.external_texture(), b.GroupAndBinding(0, 1));
    b.Global("v2", b.ty.i32(), b.GroupAndBinding(0, 2), kUniform);
    b.Global("v3", b.ty.external_texture(), b.GroupAndBinding(0, 3));
    b.Global("v4", b.ty.i32(), b.GroupAndBinding(0, 4), kUniform);
    b.WrapInFunction();

    tint::Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    auto bindings = GenerateExternalTextureBindings(&program);
    ASSERT_EQ(bindings.size(), 2u);

    auto to0 = bindings[transform::BindingPoint{0, 1}];
    ASSERT_EQ(to0.plane_1.group, 0u);
    ASSERT_EQ(to0.params.group, 0u);
    ASSERT_EQ(to0.plane_1.binding, 5u);
    ASSERT_EQ(to0.params.binding, 6u);

    auto to1 = bindings[transform::BindingPoint{0, 3}];
    ASSERT_EQ(to1.plane_1.group, 0u);
    ASSERT_EQ(to1.params.group, 0u);
    ASSERT_EQ(to1.plane_1.binding, 7u);
    ASSERT_EQ(to1.params.binding, 8u);
}

}  // namespace
}  // namespace tint::writer
