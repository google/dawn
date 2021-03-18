// Copyright 2021 The Tint Authors.
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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/resolver/resolver_test_helper.h"

namespace tint {
namespace resolver {
namespace {

using ResolverAssignmentValidationTest = ResolverTest;

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypes) {
  // {
  //  var a : i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(var, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignThroughPointerWrongeStoreType_Fail) {
  // var a : f32;
  // const b : ptr<function,f32> = a;
  // b = 2;
  const auto priv = ast::StorageClass::kFunction;
  auto* var_a = Var("a", ty.f32(), priv);
  auto* var_b = Const("b", ty.pointer<float>(priv), Expr("a"), {});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs);
  WrapInFunction(var_a, var_b, assign);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'i32' to a variable of type 'f32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a : i32 = 2;
  //  a = 2
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });
  WrapInFunction(var, body);

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverAssignmentValidationTest,
       AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a : i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });
  WrapInFunction(var, block);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest,
       AssignIncompatibleTypesInNestedBlockStatement_Fail) {
  // {
  //  {
  //   var a : i32 = 2;
  //   a = 2.3;
  //  }
  // }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* inner_block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{{12, 34}}, lhs, rhs),
  });

  auto* outer_block = create<ast::BlockStatement>(ast::StatementList{
      inner_block,
  });

  WrapInFunction(var, outer_block);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: invalid assignment: cannot assign value of type 'f32' to a variable of type 'i32')");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
