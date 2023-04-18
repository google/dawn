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

#include "src/tint/symbol.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using SymbolTest = testing::Test;

TEST_F(SymbolTest, ToStr) {
    Symbol sym(1, ProgramID::New(), "");
    EXPECT_EQ("$1", sym.to_str());
}

TEST_F(SymbolTest, CopyAssign) {
    Symbol sym1(1, ProgramID::New(), "");
    Symbol sym2;

    EXPECT_FALSE(sym2.IsValid());
    sym2 = sym1;
    EXPECT_TRUE(sym2.IsValid());
    EXPECT_EQ(sym2, sym1);
}

TEST_F(SymbolTest, Comparison) {
    auto program_id = ProgramID::New();
    Symbol sym1(1, program_id, "1");
    Symbol sym2(2, program_id, "2");
    Symbol sym3(1, program_id, "3");

    EXPECT_TRUE(sym1 == sym3);
    EXPECT_FALSE(sym1 != sym3);
    EXPECT_FALSE(sym1 == sym2);
    EXPECT_TRUE(sym1 != sym2);
    EXPECT_FALSE(sym3 == sym2);
    EXPECT_TRUE(sym3 != sym2);
}

}  // namespace
}  // namespace tint
