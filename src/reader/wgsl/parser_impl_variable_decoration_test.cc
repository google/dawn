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

#include "gtest/gtest.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, VariableDecoration_Location) {
  ParserImpl p{"location 4"};
  auto deco = p.variable_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p.has_error());
  ASSERT_TRUE(deco->IsLocation());

  auto loc = deco->AsLocation();
  EXPECT_EQ(loc->value(), 4);
}

TEST_F(ParserImplTest, VariableDecoration_Location_MissingValue) {
  ParserImpl p{"location"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:9: invalid value for location decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Location_MissingInvalid) {
  ParserImpl p{"location nan"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:10: invalid value for location decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Builtin) {
  ParserImpl p{"builtin frag_depth"};
  auto deco = p.variable_decoration();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(deco, nullptr);
  ASSERT_TRUE(deco->IsBuiltin());

  auto builtin = deco->AsBuiltin();
  EXPECT_EQ(builtin->value(), ast::Builtin::kFragDepth);
}

TEST_F(ParserImplTest, VariableDecoration_Builtin_MissingValue) {
  ParserImpl p{"builtin"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:8: invalid value for builtin decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Builtin_MissingInvalid) {
  ParserImpl p{"builtin 3"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:9: invalid value for builtin decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Binding) {
  ParserImpl p{"binding 4"};
  auto deco = p.variable_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p.has_error());
  ASSERT_TRUE(deco->IsBinding());

  auto binding = deco->AsBinding();
  EXPECT_EQ(binding->value(), 4);
}

TEST_F(ParserImplTest, VariableDecoration_Binding_MissingValue) {
  ParserImpl p{"binding"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:8: invalid value for binding decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Binding_MissingInvalid) {
  ParserImpl p{"binding nan"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:9: invalid value for binding decoration");
}

TEST_F(ParserImplTest, VariableDecoration_set) {
  ParserImpl p{"set 4"};
  auto deco = p.variable_decoration();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(deco.get(), nullptr);
  ASSERT_TRUE(deco->IsSet());

  auto set = deco->AsSet();
  EXPECT_EQ(set->value(), 4);
}

TEST_F(ParserImplTest, VariableDecoration_Set_MissingValue) {
  ParserImpl p{"set"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:4: invalid value for set decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Set_MissingInvalid) {
  ParserImpl p{"set nan"};
  auto deco = p.variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:5: invalid value for set decoration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
