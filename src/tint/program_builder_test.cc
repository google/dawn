// Copyright 2021 The Tint Authors.
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

#include "src/tint/program_builder.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using ProgramBuilderTest = testing::Test;

TEST_F(ProgramBuilderTest, IDsAreUnique) {
    Program program_a(ProgramBuilder{});
    Program program_b(ProgramBuilder{});
    Program program_c(ProgramBuilder{});
    EXPECT_NE(program_a.ID(), program_b.ID());
    EXPECT_NE(program_b.ID(), program_c.ID());
    EXPECT_NE(program_c.ID(), program_a.ID());
}

TEST_F(ProgramBuilderTest, WrapDoesntAffectInner) {
    Program inner([] {
        ProgramBuilder builder;
        auto ty = builder.ty.f32();
        builder.Func("a", {}, ty, {}, {});
        return builder;
    }());

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_TRUE(inner.Symbols().Get("a").IsValid());
    ASSERT_FALSE(inner.Symbols().Get("b").IsValid());

    ProgramBuilder outer = ProgramBuilder::Wrap(&inner);

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_EQ(outer.AST().Functions().Length(), 1u);
    EXPECT_EQ(inner.AST().Functions()[0], outer.AST().Functions()[0]);
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_EQ(inner.Symbols().Get("a"), outer.Symbols().Get("a"));
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("a").IsValid());
    EXPECT_FALSE(inner.Symbols().Get("b").IsValid());
    EXPECT_FALSE(outer.Symbols().Get("b").IsValid());

    auto ty = outer.ty.f32();
    outer.Func("b", {}, ty, {}, {});

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_EQ(outer.AST().Functions().Length(), 2u);
    EXPECT_EQ(inner.AST().Functions()[0], outer.AST().Functions()[0]);
    EXPECT_EQ(outer.AST().Functions()[1]->name->symbol, outer.Symbols().Get("b"));
    EXPECT_EQ(inner.Symbols().Get("a"), outer.Symbols().Get("a"));
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("a").IsValid());
    EXPECT_FALSE(inner.Symbols().Get("b").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("b").IsValid());
}

}  // namespace
}  // namespace tint
