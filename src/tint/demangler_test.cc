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

#include "src/tint/demangler.h"
#include "src/tint/symbol_table.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using DemanglerTest = testing::Test;

TEST_F(DemanglerTest, NoSymbols) {
    SymbolTable t{ProgramID::New()};
    t.Register("sym1");

    Demangler d;
    EXPECT_EQ("test str", d.Demangle(t, "test str"));
}

TEST_F(DemanglerTest, Symbol) {
    SymbolTable t{ProgramID::New()};
    t.Register("sym1");

    Demangler d;
    EXPECT_EQ("test sym1 str", d.Demangle(t, "test $1 str"));
}

TEST_F(DemanglerTest, MultipleSymbols) {
    SymbolTable t{ProgramID::New()};
    t.Register("sym1");
    t.Register("sym2");

    Demangler d;
    EXPECT_EQ("test sym1 sym2 sym1 str", d.Demangle(t, "test $1 $2 $1 str"));
}

}  // namespace
}  // namespace tint
