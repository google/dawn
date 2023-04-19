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

#include "src/tint/writer/flatten_bindings.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::writer {
namespace {

using namespace tint::number_suffixes;  // NOLINT

class FlattenBindingsTest : public ::testing::Test {};

TEST_F(FlattenBindingsTest, NoBindings) {
    ProgramBuilder b;
    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, AlreadyFlat) {
    ProgramBuilder b;
    b.GlobalVar("a", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("b", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(1_a));
    b.GlobalVar("c", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(2_a));

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, NotFlat_SingleNamespace) {
    ProgramBuilder b;
    b.GlobalVar("a", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("b", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(1_a), b.Binding(1_a));
    b.GlobalVar("c", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(2_a), b.Binding(2_a));
    b.WrapInFunction(b.Expr("a"), b.Expr("b"), b.Expr("c"));

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[0]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint()->group, 0u);
    EXPECT_EQ(sem->BindingPoint()->binding, 0u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[1]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint()->group, 0u);
    EXPECT_EQ(sem->BindingPoint()->binding, 1u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[2]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint()->group, 0u);
    EXPECT_EQ(sem->BindingPoint()->binding, 2u);
}

TEST_F(FlattenBindingsTest, NotFlat_MultipleNamespaces) {
    ProgramBuilder b;

    const size_t num_buffers = 3;
    b.GlobalVar("buffer1", b.ty.i32(), builtin::AddressSpace::kUniform, b.Group(0_a),
                b.Binding(0_a));
    b.GlobalVar("buffer2", b.ty.i32(), builtin::AddressSpace::kStorage, b.Group(1_a),
                b.Binding(1_a));
    b.GlobalVar("buffer3", b.ty.i32(), builtin::AddressSpace::kStorage, builtin::Access::kRead,
                b.Group(2_a), b.Binding(2_a));

    const size_t num_samplers = 2;
    b.GlobalVar("sampler1", b.ty.sampler(type::SamplerKind::kSampler), b.Group(3_a),
                b.Binding(3_a));
    b.GlobalVar("sampler2", b.ty.sampler(type::SamplerKind::kComparisonSampler), b.Group(4_a),
                b.Binding(4_a));

    const size_t num_textures = 6;
    b.GlobalVar("texture1", b.ty.sampled_texture(type::TextureDimension::k2d, b.ty.f32()),
                b.Group(5_a), b.Binding(5_a));
    b.GlobalVar("texture2", b.ty.multisampled_texture(type::TextureDimension::k2d, b.ty.f32()),
                b.Group(6_a), b.Binding(6_a));
    b.GlobalVar("texture3",
                b.ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Float,
                                     builtin::Access::kWrite),
                b.Group(7_a), b.Binding(7_a));
    b.GlobalVar("texture4", b.ty.depth_texture(type::TextureDimension::k2d), b.Group(8_a),
                b.Binding(8_a));
    b.GlobalVar("texture5", b.ty.depth_multisampled_texture(type::TextureDimension::k2d),
                b.Group(9_a), b.Binding(9_a));
    b.GlobalVar("texture6", b.ty.external_texture(), b.Group(10_a), b.Binding(10_a));

    b.WrapInFunction(b.Assign(b.Phony(), "buffer1"), b.Assign(b.Phony(), "buffer2"),
                     b.Assign(b.Phony(), "buffer3"), b.Assign(b.Phony(), "sampler1"),
                     b.Assign(b.Phony(), "sampler2"), b.Assign(b.Phony(), "texture1"),
                     b.Assign(b.Phony(), "texture2"), b.Assign(b.Phony(), "texture3"),
                     b.Assign(b.Phony(), "texture4"), b.Assign(b.Phony(), "texture5"),
                     b.Assign(b.Phony(), "texture6"));

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    for (size_t i = 0; i < num_buffers; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->BindingPoint()->group, 0u);
        EXPECT_EQ(sem->BindingPoint()->binding, i);
    }
    for (size_t i = 0; i < num_samplers; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->BindingPoint()->group, 0u);
        EXPECT_EQ(sem->BindingPoint()->binding, i);
    }
    for (size_t i = 0; i < num_textures; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers + num_samplers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->BindingPoint()->group, 0u);
        EXPECT_EQ(sem->BindingPoint()->binding, i);
    }
}

}  // namespace
}  // namespace tint::writer
