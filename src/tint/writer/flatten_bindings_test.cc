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
#include "src/tint/resolver/resolver.h"
#include "src/tint/sem/variable.h"

namespace tint::writer {
namespace {

class FlattenBindingsTest : public ::testing::Test {};

TEST_F(FlattenBindingsTest, NoBindings) {
    ProgramBuilder b;
    b.WrapInFunction();

    resolver::Resolver resolver(&b);

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, AlreadyFlat) {
    ProgramBuilder b;
    b.Global("a", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(0, 0));
    b.Global("b", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(0, 1));
    b.Global("c", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(0, 2));
    b.WrapInFunction();

    resolver::Resolver resolver(&b);

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, NotFlat_SingleNamespace) {
    ProgramBuilder b;
    b.Global("a", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(0, 0));
    b.Global("b", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(1, 1));
    b.Global("c", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(2, 2));
    b.WrapInFunction(b.Expr("a"), b.Expr("b"), b.Expr("c"));

    resolver::Resolver resolver(&b);

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();
    EXPECT_EQ(vars[0]->BindingPoint().group->value, 0u);
    EXPECT_EQ(vars[0]->BindingPoint().binding->value, 0u);
    EXPECT_EQ(vars[1]->BindingPoint().group->value, 0u);
    EXPECT_EQ(vars[1]->BindingPoint().binding->value, 1u);
    EXPECT_EQ(vars[2]->BindingPoint().group->value, 0u);
    EXPECT_EQ(vars[2]->BindingPoint().binding->value, 2u);
}

TEST_F(FlattenBindingsTest, NotFlat_MultipleNamespaces) {
    ProgramBuilder b;

    const size_t num_buffers = 3;
    b.Global("buffer1", b.ty.i32(), ast::StorageClass::kUniform, b.GroupAndBinding(0, 0));
    b.Global("buffer2", b.ty.i32(), ast::StorageClass::kStorage, b.GroupAndBinding(1, 1));
    b.Global("buffer3", b.ty.i32(), ast::StorageClass::kStorage, ast::Access::kRead,
             b.GroupAndBinding(2, 2));

    const size_t num_samplers = 2;
    b.Global("sampler1", b.ty.sampler(ast::SamplerKind::kSampler), b.GroupAndBinding(3, 3));
    b.Global("sampler2", b.ty.sampler(ast::SamplerKind::kComparisonSampler),
             b.GroupAndBinding(4, 4));

    const size_t num_textures = 6;
    b.Global("texture1", b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
             b.GroupAndBinding(5, 5));
    b.Global("texture2", b.ty.multisampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
             b.GroupAndBinding(6, 6));
    b.Global("texture3",
             b.ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                  ast::Access::kWrite),
             b.GroupAndBinding(7, 7));
    b.Global("texture4", b.ty.depth_texture(ast::TextureDimension::k2d), b.GroupAndBinding(8, 8));
    b.Global("texture5", b.ty.depth_multisampled_texture(ast::TextureDimension::k2d),
             b.GroupAndBinding(9, 9));
    b.Global("texture6", b.ty.external_texture(), b.GroupAndBinding(10, 10));

    b.WrapInFunction(b.Assign(b.Phony(), "buffer1"), b.Assign(b.Phony(), "buffer2"),
                     b.Assign(b.Phony(), "buffer3"), b.Assign(b.Phony(), "sampler1"),
                     b.Assign(b.Phony(), "sampler2"), b.Assign(b.Phony(), "texture1"),
                     b.Assign(b.Phony(), "texture2"), b.Assign(b.Phony(), "texture3"),
                     b.Assign(b.Phony(), "texture4"), b.Assign(b.Phony(), "texture5"),
                     b.Assign(b.Phony(), "texture6"));

    resolver::Resolver resolver(&b);

    Program program(std::move(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    auto flattened = tint::writer::FlattenBindings(&program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    for (size_t i = 0; i < num_buffers; ++i) {
        EXPECT_EQ(vars[i]->BindingPoint().group->value, 0u);
        EXPECT_EQ(vars[i]->BindingPoint().binding->value, i);
    }
    for (size_t i = 0; i < num_samplers; ++i) {
        EXPECT_EQ(vars[i + num_buffers]->BindingPoint().group->value, 0u);
        EXPECT_EQ(vars[i + num_buffers]->BindingPoint().binding->value, i);
    }
    for (size_t i = 0; i < num_textures; ++i) {
        EXPECT_EQ(vars[i + num_buffers + num_samplers]->BindingPoint().group->value, 0u);
        EXPECT_EQ(vars[i + num_buffers + num_samplers]->BindingPoint().binding->value, i);
    }
}

}  // namespace
}  // namespace tint::writer
