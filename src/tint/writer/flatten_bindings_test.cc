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

namespace tint::writer {
namespace {

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
    b.GlobalVar("a", b.ty.i32(), ast::StorageClass::kUniform, b.Group(0), b.Binding(0));
    b.GlobalVar("b", b.ty.i32(), ast::StorageClass::kUniform, b.Group(0), b.Binding(1));
    b.GlobalVar("c", b.ty.i32(), ast::StorageClass::kUniform, b.Group(0), b.Binding(2));

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, NotFlat_SingleNamespace) {
    ProgramBuilder b;
    b.GlobalVar("a", b.ty.i32(), ast::StorageClass::kUniform, b.Group(0), b.Binding(0));
    b.GlobalVar("b", b.ty.i32(), ast::StorageClass::kUniform, b.Group(1), b.Binding(1));
    b.GlobalVar("c", b.ty.i32(), ast::StorageClass::kUniform, b.Group(2), b.Binding(2));
    b.WrapInFunction(b.Expr("a"), b.Expr("b"), b.Expr("c"));

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[0]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint().group, 0u);
    EXPECT_EQ(sem->BindingPoint().binding, 0u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[1]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint().group, 0u);
    EXPECT_EQ(sem->BindingPoint().binding, 1u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[2]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->BindingPoint().group, 0u);
    EXPECT_EQ(sem->BindingPoint().binding, 2u);
}

TEST_F(FlattenBindingsTest, NotFlat_MultipleNamespaces) {
    ProgramBuilder b;

    const size_t num_buffers = 3;
    b.GlobalVar("buffer1", b.ty.i32(), ast::StorageClass::kUniform, b.Group(0), b.Binding(0));
    b.GlobalVar("buffer2", b.ty.i32(), ast::StorageClass::kStorage, b.Group(1), b.Binding(1));
    b.GlobalVar("buffer3", b.ty.i32(), ast::StorageClass::kStorage, ast::Access::kRead, b.Group(2),
                b.Binding(2));

    const size_t num_samplers = 2;
    b.GlobalVar("sampler1", b.ty.sampler(ast::SamplerKind::kSampler), b.Group(3), b.Binding(3));
    b.GlobalVar("sampler2", b.ty.sampler(ast::SamplerKind::kComparisonSampler), b.Group(4),
                b.Binding(4));

    const size_t num_textures = 6;
    b.GlobalVar("texture1", b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
                b.Group(5), b.Binding(5));
    b.GlobalVar("texture2", b.ty.multisampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
                b.Group(6), b.Binding(6));
    b.GlobalVar("texture3",
                b.ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                     ast::Access::kWrite),
                b.Group(7), b.Binding(7));
    b.GlobalVar("texture4", b.ty.depth_texture(ast::TextureDimension::k2d), b.Group(8),
                b.Binding(8));
    b.GlobalVar("texture5", b.ty.depth_multisampled_texture(ast::TextureDimension::k2d), b.Group(9),
                b.Binding(9));
    b.GlobalVar("texture6", b.ty.external_texture(), b.Group(10), b.Binding(10));

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
        EXPECT_EQ(sem->BindingPoint().group, 0u);
        EXPECT_EQ(sem->BindingPoint().binding, i);
    }
    for (size_t i = 0; i < num_samplers; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->BindingPoint().group, 0u);
        EXPECT_EQ(sem->BindingPoint().binding, i);
    }
    for (size_t i = 0; i < num_textures; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers + num_samplers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->BindingPoint().group, 0u);
        EXPECT_EQ(sem->BindingPoint().binding, i);
    }
}

}  // namespace
}  // namespace tint::writer
