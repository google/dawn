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

#include "gtest/gtest-spi.h"
#include "src/tint/debug.h"
#include "src/tint/program_builder.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/utils/get_insertion_point.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {
namespace {

using GetInsertionPointTest = ::testing::Test;

TEST_F(GetInsertionPointTest, Block) {
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* block = b.Block(var);
    b.Func("f", tint::utils::Empty, b.ty.void_(), tint::utils::Vector{block});

    Program original(std::move(b));
    ProgramBuilder cloned_b;
    CloneContext ctx(&cloned_b, &original);

    // Can insert in block containing the variable, above or below the input
    // statement.
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first->Declaration(), block);
    ASSERT_EQ(ip.second, var);
}

TEST_F(GetInsertionPointTest, ForLoopInit) {
    // fn f() {
    //     for(var a = 1i; true; ) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* fl = b.For(var, b.Expr(true), nullptr, b.Block());
    auto* func_block = b.Block(fl);
    b.Func("f", tint::utils::Empty, b.ty.void_(), tint::utils::Vector{func_block});

    Program original(std::move(b));
    ProgramBuilder cloned_b;
    CloneContext ctx(&cloned_b, &original);

    // Can insert in block containing for-loop above the for-loop itself.
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first->Declaration(), func_block);
    ASSERT_EQ(ip.second, fl);
}

TEST_F(GetInsertionPointTest, ForLoopCont_Invalid) {
    // fn f() {
    //     for(; true; var a = 1i) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* s = b.For({}, b.Expr(true), var, b.Block());
    b.Func("f", tint::utils::Empty, b.ty.void_(), tint::utils::Vector{s});

    Program original(std::move(b));
    ProgramBuilder cloned_b;
    CloneContext ctx(&cloned_b, &original);

    // Can't insert before/after for loop continue statement (would ned to be
    // converted to loop).
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first, nullptr);
    ASSERT_EQ(ip.second, nullptr);
}

}  // namespace
}  // namespace tint::transform
