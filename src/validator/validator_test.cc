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

#include "src/ast/if_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidatorTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, nullptr);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  WrapInFunction(body);

  ValidatorImpl& v = Build();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(v.ValidateStatements(body)) << v.error();
}

}  // namespace
}  // namespace tint
