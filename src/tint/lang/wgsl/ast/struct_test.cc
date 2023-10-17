// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/struct.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"

namespace tint::ast {
namespace {

using AstStructTest = TestHelper;
using BlockAttribute = transform::AddBlockAttribute::BlockAttribute;

TEST_F(AstStructTest, Creation) {
    auto name = Sym("s");
    auto* s = Structure(name, tint::Vector{Member("a", ty.i32())});
    EXPECT_EQ(s->name->symbol, name);
    EXPECT_EQ(s->members.Length(), 1u);
    EXPECT_TRUE(s->attributes.IsEmpty());
    EXPECT_EQ(s->source.range.begin.line, 0u);
    EXPECT_EQ(s->source.range.begin.column, 0u);
    EXPECT_EQ(s->source.range.end.line, 0u);
    EXPECT_EQ(s->source.range.end.column, 0u);
}

TEST_F(AstStructTest, Creation_WithAttributes) {
    auto name = Sym("s");

    auto* s = Structure(name, tint::Vector{Member("a", ty.i32())},
                        tint::Vector{
                            ASTNodes().Create<BlockAttribute>(ID(), AllocateNodeID()),
                        });
    EXPECT_EQ(s->name->symbol, name);
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
    auto* s = Structure(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
                        name, tint::Vector{Member("a", ty.i32())},
                        tint::Vector{ASTNodes().Create<BlockAttribute>(ID(), AllocateNodeID())});
    EXPECT_EQ(s->name->symbol, name);
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
            b.Structure(b.Sym("S"), tint::Vector{b.Member("a", b.ty.i32()), nullptr}, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_Null_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Structure(b.Sym("S"), tint::Vector{b.Member("a", b.ty.i32())},
                        tint::Vector<const Attribute*, 1>{nullptr});
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentGenerationID_StructMember) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Structure(b1.Sym("S"), tint::Vector{b2.Member("a", b2.ty.i32())});
        },
        "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentGenerationID_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Structure(
                b1.Sym("S"), tint::Vector{b1.Member("a", b1.ty.i32())},
                tint::Vector{b2.ASTNodes().Create<BlockAttribute>(b2.ID(), b2.AllocateNodeID())});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
