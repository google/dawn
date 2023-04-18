// Copyright 2020 The Tint Authors.  //
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

#include "src/tint/scope_stack.h"

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"

namespace tint {
namespace {

class ScopeStackTest : public ProgramBuilder, public testing::Test {};

TEST_F(ScopeStackTest, Get) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(3, ID(), "3");
    s.Push();
    s.Set(a, 5u);
    s.Set(b, 10u);

    EXPECT_EQ(s.Get(a), 5u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Push();

    s.Set(a, 15u);
    EXPECT_EQ(s.Get(a), 15u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Pop();
    EXPECT_EQ(s.Get(a), 5u);
    EXPECT_EQ(s.Get(b), 10u);
}

TEST_F(ScopeStackTest, Get_MissingSymbol) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol sym(1, ID(), "1");
    EXPECT_EQ(s.Get(sym), 0u);
}

TEST_F(ScopeStackTest, Set) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(2, ID(), "2");

    EXPECT_EQ(s.Set(a, 5u), 0u);
    EXPECT_EQ(s.Get(a), 5u);

    EXPECT_EQ(s.Set(b, 10u), 0u);
    EXPECT_EQ(s.Get(b), 10u);

    EXPECT_EQ(s.Set(a, 20u), 5u);
    EXPECT_EQ(s.Get(a), 20u);

    EXPECT_EQ(s.Set(b, 25u), 10u);
    EXPECT_EQ(s.Get(b), 25u);
}

TEST_F(ScopeStackTest, Clear) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(2, ID(), "2");

    EXPECT_EQ(s.Set(a, 5u), 0u);
    EXPECT_EQ(s.Get(a), 5u);

    s.Push();

    EXPECT_EQ(s.Set(b, 10u), 0u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Push();

    s.Clear();
    EXPECT_EQ(s.Get(a), 0u);
    EXPECT_EQ(s.Get(b), 0u);
}

}  // namespace
}  // namespace tint
