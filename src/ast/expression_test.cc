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

#include "src/ast/expression.h"

#include "gtest/gtest.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

class Expr : public Expression {
 public:
  Expr() : Expression() {}

  bool IsValid() const override { return true; }
  void to_str(std::ostream&, size_t) const override {}
};

using ExpressionTest = testing::Test;

TEST_F(ExpressionTest, set_result_type) {
  type::I32Type i32;

  Expr e;
  e.set_result_type(&i32);
  ASSERT_NE(e.result_type(), nullptr);
  EXPECT_TRUE(e.result_type()->IsI32());
}

TEST_F(ExpressionTest, set_result_type_alias) {
  type::I32Type i32;
  type::AliasType a("a", &i32);
  type::AliasType b("b", &a);

  Expr e;
  e.set_result_type(&b);
  ASSERT_NE(e.result_type(), nullptr);
  EXPECT_TRUE(e.result_type()->IsI32());
}

}  // namespace
}  // namespace ast
}  // namespace tint
