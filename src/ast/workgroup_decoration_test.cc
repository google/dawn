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

#include "src/ast/workgroup_decoration.h"

#include "src/ast/stage_decoration.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using WorkgroupDecorationTest = TestHelper;

TEST_F(WorkgroupDecorationTest, Creation_1param) {
  auto* d = WorkgroupSize(2);
  auto values = d->values();
  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(x_scalar);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  EXPECT_EQ(values[1], nullptr);
  EXPECT_EQ(values[2], nullptr);
}
TEST_F(WorkgroupDecorationTest, Creation_2param) {
  auto* d = WorkgroupSize(2, 4);
  auto values = d->values();
  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(x_scalar);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(y_scalar);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  EXPECT_EQ(values[2], nullptr);
}

TEST_F(WorkgroupDecorationTest, Creation_3param) {
  auto* d = WorkgroupSize(2, 4, 6);
  auto values = d->values();
  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(x_scalar);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(y_scalar);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_NE(values[2], nullptr);
  auto* z_scalar = values[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(z_scalar);
  ASSERT_TRUE(z_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(z_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 6u);
}

TEST_F(WorkgroupDecorationTest, Creation_WithIdentifier) {
  auto* d = WorkgroupSize(2, 4, "depth");
  auto values = d->values();
  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(x_scalar);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(y_scalar);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_NE(values[2], nullptr);
  auto* z_ident = values[2]->As<ast::IdentifierExpression>();
  ASSERT_TRUE(z_ident);
  EXPECT_EQ(Symbols().NameFor(z_ident->symbol()), "depth");
}

TEST_F(WorkgroupDecorationTest, ToStr) {
  auto* d = WorkgroupSize(2, "height");
  EXPECT_EQ(str(d), R"(WorkgroupDecoration{
  ScalarConstructor[not set]{2}
  Identifier[not set]{height}
}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
