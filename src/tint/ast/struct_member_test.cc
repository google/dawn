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

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using StructMemberTest = TestHelper;

TEST_F(StructMemberTest, Creation) {
    auto* st = Member("a", ty.i32(), utils::Vector{MemberSize(4_a)});
    CheckIdentifier(st->name, "a");
    CheckIdentifier(st->type, "i32");
    EXPECT_EQ(st->attributes.Length(), 1u);
    EXPECT_TRUE(st->attributes[0]->Is<StructMemberSizeAttribute>());
    EXPECT_EQ(st->source.range.begin.line, 0u);
    EXPECT_EQ(st->source.range.begin.column, 0u);
    EXPECT_EQ(st->source.range.end.line, 0u);
    EXPECT_EQ(st->source.range.end.column, 0u);
}

TEST_F(StructMemberTest, CreationWithSource) {
    auto* st = Member(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}}, "a",
                      ty.i32());
    CheckIdentifier(st->name, "a");
    CheckIdentifier(st->type, "i32");
    EXPECT_EQ(st->attributes.Length(), 0u);
    EXPECT_EQ(st->source.range.begin.line, 27u);
    EXPECT_EQ(st->source.range.begin.column, 4u);
    EXPECT_EQ(st->source.range.end.line, 27u);
    EXPECT_EQ(st->source.range.end.column, 8u);
}

TEST_F(StructMemberTest, Assert_Null_Name) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Member(static_cast<Identifier*>(nullptr), b.ty.i32());
        },
        "internal compiler error");
}

TEST_F(StructMemberTest, Assert_Null_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Member("a", Type{});
        },
        "internal compiler error");
}

TEST_F(StructMemberTest, Assert_Null_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Member("a", b.ty.i32(), utils::Vector{b.MemberSize(4_a), nullptr});
        },
        "internal compiler error");
}

TEST_F(StructMemberTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Member(b2.Sym("a"), b1.ty.i32(), utils::Vector{b1.MemberSize(4_a)});
        },
        "internal compiler error");
}

TEST_F(StructMemberTest, Assert_DifferentProgramID_Attribute) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Member("a", b1.ty.i32(), utils::Vector{b2.MemberSize(4_a)});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
