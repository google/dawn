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
#include "src/ast/array_accessor_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ArgumentExpressionList_Parses) {
  auto* p = parser("a");
  auto e = p->argument_expression_list();
  ASSERT_FALSE(p->has_error()) << p->error();

  ASSERT_EQ(e.size(), 1u);
  ASSERT_TRUE(e[0]->IsIdentifier());
}

TEST_F(ParserImplTest, ArgumentExpressionList_ParsesMultiple) {
  auto* p = parser("a, -33, 1+2");
  auto e = p->argument_expression_list();
  ASSERT_FALSE(p->has_error()) << p->error();

  ASSERT_EQ(e.size(), 3u);
  ASSERT_TRUE(e[0]->IsIdentifier());
  ASSERT_TRUE(e[1]->IsConstructor());
  ASSERT_TRUE(e[2]->IsBinary());
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesMissingExpression) {
  auto* p = parser("a, ");
  auto e = p->argument_expression_list();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: unable to parse argument expression after comma");
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesInvalidExpression) {
  auto* p = parser("if(a) {}");
  auto e = p->argument_expression_list();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:1: unable to parse argument expression");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
