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
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, VariableDecorationList_Parses) {
  ParserImpl p{R"([[location 4, builtin position]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_FALSE(p.has_error());
  ASSERT_EQ(decos.size(), 2);
  ASSERT_TRUE(decos[0]->IsLocation());
  EXPECT_EQ(decos[0]->AsLocation()->value(), 4);
  ASSERT_TRUE(decos[1]->IsBuiltin());
  EXPECT_EQ(decos[1]->AsBuiltin()->value(), ast::Builtin::kPosition);
}

TEST_F(ParserImplTest, VariableDecorationList_Empty) {
  ParserImpl p{R"([[]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:3: empty variable decoration list");
}

TEST_F(ParserImplTest, VariableDecorationList_Invalid) {
  ParserImpl p{R"([[invalid]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:3: missing variable decoration for decoration list");
}

TEST_F(ParserImplTest, VariableDecorationList_ExtraComma) {
  ParserImpl p{R"([[builtin position, ]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:21: missing variable decoration after comma");
}

TEST_F(ParserImplTest, VariableDecorationList_MissingComma) {
  ParserImpl p{R"([[binding 4 location 5]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:13: missing comma in variable decoration list");
}

TEST_F(ParserImplTest, VariableDecorationList_BadDecoration) {
  ParserImpl p{R"([[location bad]])"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:12: invalid value for location decoration");
}

TEST_F(ParserImplTest, VariableDecorationList_InvalidBuiltin) {
  ParserImpl p{"[[builtin invalid]]"};
  auto decos = p.variable_decoration_list();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:11: invalid value for builtin decoration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
