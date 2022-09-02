// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/struct.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/ast/texture.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/vector.h"
#include "src/tint/transform/add_block_attribute.h"

namespace tint::ast {
namespace {

using AstStructTest = TestHelper;
using BlockAttribute = transform::AddBlockAttribute::BlockAttribute;

TEST_F(AstStructTest, Creation) {
    auto name = Sym("s");
    auto* s = create<Struct>(name, utils::Vector{Member("a", ty.i32())}, utils::Empty);
    EXPECT_EQ(s->name, name);
    EXPECT_EQ(s->members.Length(), 1u);
    EXPECT_TRUE(s->attributes.IsEmpty());
    EXPECT_EQ(s->source.range.begin.line, 0u);
    EXPECT_EQ(s->source.range.begin.column, 0u);
    EXPECT_EQ(s->source.range.end.line, 0u);
    EXPECT_EQ(s->source.range.end.column, 0u);
}

TEST_F(AstStructTest, Creation_WithAttributes) {
    auto name = Sym("s");

    auto* s = create<Struct>(name, utils::Vector{Member("a", ty.i32())},
                             utils::Vector{
                                 ASTNodes().Create<BlockAttribute>(ID(), AllocateNodeID()),
                             });
    EXPECT_EQ(s->name, name);
    EXPECT_EQ(s->members.Length(), 1u);
    ASSERT_EQ(s->attributes.Length(), 1u);
    EXPECT_TRUE(s->attributes[0]->Is<BlockAttribute>());
    EXPECT_EQ(s->source.range.begin.line, 0u);
    EXPECT_EQ(s->source.range.begin.column, 0u);
    EXPECT_EQ(s->source.range.end.line, 0u);
    EXPECT_EQ(s->source.range.end.column, 0u);
}

TEST_F(AstStructTest, CreationWithSourceAndAttributes) {
    auto name = Sym("s");
    auto* s =
        create<Struct>(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
                       name, utils::Vector{Member("a", ty.i32())},
                       utils::Vector{ASTNodes().Create<BlockAttribute>(ID(), AllocateNodeID())});
    EXPECT_EQ(s->name, name);
    EXPECT_EQ(s->members.Length(), 1u);
    ASSERT_EQ(s->attributes.Length(), 1u);
    EXPECT_TRUE(s->attributes[0]->Is<BlockAttribute>());
    EXPECT_EQ(s->source.range.begin.line, 27u);
    EXPECT_EQ(s->source.range.begin.column, 4u);
    EXPECT_EQ(s->source.range.end.line, 27u);
    EXPECT_EQ(s->source.range.end.column, 8u);
}

TEST_F(AstStructTest, Assert_Null_StructMember) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<Struct>(b.Sym("S"), utils::Vector{b.Member("a", b.ty.i32()), nullptr},
                             utils::Empty);
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_Null_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<Struct>(b.Sym("S"), utils::Vector{b.Member("a", b.ty.i32())},
                             utils::Vector<const ast::Attribute*, 1>{nullptr});
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentProgramID_StructMember) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<Struct>(b1.Sym("S"), utils::Vector{b2.Member("a", b2.ty.i32())},
                              utils::Empty);
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentProgramID_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<Struct>(
                b1.Sym("S"), utils::Vector{b1.Member("a", b1.ty.i32())},
                utils::Vector{b2.ASTNodes().Create<BlockAttribute>(b2.ID(), b2.AllocateNodeID())});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
