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

#include "src/namer.h"

#include "gtest/gtest.h"
#include "src/symbol_table.h"

namespace tint {
namespace {

using NamerTest = testing::Test;

TEST_F(NamerTest, GenerateName) {
  SymbolTable t;
  MangleNamer n(&t);
  EXPECT_EQ("name", n.GenerateName("name"));
  EXPECT_EQ("name_0", n.GenerateName("name"));
  EXPECT_EQ("name_1", n.GenerateName("name"));
}

using MangleNamerTest = testing::Test;

TEST_F(MangleNamerTest, ReturnsName) {
  SymbolTable t;
  auto s = t.Register("my_sym");

  MangleNamer n(&t);
  EXPECT_EQ("tint_symbol_1", n.NameFor(s));
}

TEST_F(MangleNamerTest, ReturnsSameValueForSameName) {
  SymbolTable t;
  auto s1 = t.Register("my_sym");
  auto s2 = t.Register("my_sym2");

  MangleNamer n(&t);
  EXPECT_EQ("tint_symbol_1", n.NameFor(s1));
  EXPECT_EQ("tint_symbol_2", n.NameFor(s2));
  EXPECT_EQ("tint_symbol_1", n.NameFor(s1));
}

using UnsafeNamerTest = testing::Test;
TEST_F(UnsafeNamerTest, ReturnsName) {
  SymbolTable t;
  auto s = t.Register("my_sym");

  UnsafeNamer n(&t);
  EXPECT_EQ("my_sym", n.NameFor(s));
}

}  // namespace
}  // namespace tint
