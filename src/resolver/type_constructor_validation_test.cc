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

#include "src/resolver/resolver_test_helper.h"
#include "src/sem/reference_type.h"

namespace tint {
namespace resolver {
namespace {

/// @return the element type of `type` for vec and mat, otherwise `type` itself
ast::Type* ElementTypeOf(ast::Type* type) {
  if (auto* v = type->As<ast::Vector>()) {
    return v->type();
  }
  if (auto* m = type->As<ast::Matrix>()) {
    return m->type();
  }
  return type;
}

class ResolverTypeConstructorValidationTest : public resolver::TestHelper,
                                              public testing::Test {};

namespace InferTypeTest {
struct Params {
  create_ast_type_func_ptr create_rhs_ast_type;
  create_sem_type_func_ptr create_rhs_sem_type;
};

// Helpers and typedefs
using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;

TEST_F(ResolverTypeConstructorValidationTest, InferTypeTest_Simple) {
  // var a = 1;
  // var b = a;
  auto* a = Var("a", nullptr, ast::StorageClass::kNone, Expr(1));
  auto* b = Var("b", nullptr, ast::StorageClass::kNone, Expr("a"));
  auto* a_ident = Expr("a");
  auto* b_ident = Expr("b");

  WrapInFunction(a, b, Assign(a_ident, "a"), Assign(b_ident, "b"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_TRUE(TypeOf(a_ident)->Is<sem::Reference>());
  EXPECT_TRUE(
      TypeOf(a_ident)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(a_ident)->As<sem::Reference>()->StorageClass(),
            ast::StorageClass::kFunction);
  ASSERT_TRUE(TypeOf(b_ident)->Is<sem::Reference>());
  EXPECT_TRUE(
      TypeOf(b_ident)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(b_ident)->As<sem::Reference>()->StorageClass(),
            ast::StorageClass::kFunction);
}

using InferTypeTest_FromConstructorExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromConstructorExpression, All) {
  // e.g. for vec3<f32>
  // {
  //   var a = vec3<f32>(0.0, 0.0, 0.0)
  // }
  auto& params = GetParam();

  auto* rhs_type = params.create_rhs_ast_type(ty);
  auto* constructor_expr = ConstructValueFilledWith(rhs_type, 0);

  auto* a = Var("a", nullptr, ast::StorageClass::kNone, constructor_expr);
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(ty),
                                          ast::StorageClass::kFunction);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}

static constexpr Params from_constructor_expression_cases[] = {
    Params{ast_bool, sem_bool},
    Params{ast_i32, sem_i32},
    Params{ast_u32, sem_u32},
    Params{ast_f32, sem_f32},
    Params{ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{ast_mat3x3<i32>, sem_mat3x3<sem_i32>},
    Params{ast_mat3x3<u32>, sem_mat3x3<sem_u32>},
    Params{ast_mat3x3<f32>, sem_mat3x3<sem_f32>},
    Params{ast_alias<ast_bool>, sem_bool},
    Params{ast_alias<ast_i32>, sem_i32},
    Params{ast_alias<ast_u32>, sem_u32},
    Params{ast_alias<ast_f32>, sem_f32},
    Params{ast_alias<ast_vec3<i32>>, sem_vec3<sem_i32>},
    Params{ast_alias<ast_vec3<u32>>, sem_vec3<sem_u32>},
    Params{ast_alias<ast_vec3<f32>>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_mat3x3<i32>>, sem_mat3x3<sem_i32>},
    Params{ast_alias<ast_mat3x3<u32>>, sem_mat3x3<sem_u32>},
    Params{ast_alias<ast_mat3x3<f32>>, sem_mat3x3<sem_f32>},
};
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         InferTypeTest_FromConstructorExpression,
                         testing::ValuesIn(from_constructor_expression_cases));

using InferTypeTest_FromArithmeticExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromArithmeticExpression, All) {
  // e.g. for vec3<f32>
  // {
  //   var a = vec3<f32>(2.0, 2.0, 2.0) * 3.0;
  // }
  auto& params = GetParam();

  auto* rhs_type = params.create_rhs_ast_type(ty);

  auto* arith_lhs_expr = ConstructValueFilledWith(rhs_type, 2);
  auto* arith_rhs_expr = ConstructValueFilledWith(ElementTypeOf(rhs_type), 3);
  auto* constructor_expr = Mul(arith_lhs_expr, arith_rhs_expr);

  auto* a = Var("a", nullptr, ast::StorageClass::kNone, constructor_expr);
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(ty),
                                          ast::StorageClass::kFunction);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_arithmetic_expression_cases[] = {
    Params{ast_i32, sem_i32},
    Params{ast_u32, sem_u32},
    Params{ast_f32, sem_f32},
    Params{ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{ast_mat3x3<f32>, sem_mat3x3<sem_f32>},

    // TODO(amaiorano): Uncomment once https://crbug.com/tint/680 is fixed
    // Params{ty_alias<ty_i32>},
    // Params{ty_alias<ty_u32>},
    // Params{ty_alias<ty_f32>},
    // Params{ty_alias<ty_vec3<f32>>},
    // Params{ty_alias<ty_mat3x3<f32>>},
};
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         InferTypeTest_FromArithmeticExpression,
                         testing::ValuesIn(from_arithmetic_expression_cases));

using InferTypeTest_FromCallExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromCallExpression, All) {
  // e.g. for vec3<f32>
  //
  // fn foo() -> vec3<f32> {
  //   return vec3<f32>(0.0, 0.0, 0.0);
  // }
  //
  // fn bar()
  // {
  //   var a = foo();
  // }
  auto& params = GetParam();

  Func("foo", {}, params.create_rhs_ast_type(ty),
       {Return(ConstructValueFilledWith(params.create_rhs_ast_type(ty), 0))},
       {});

  auto* a = Var("a", nullptr, ast::StorageClass::kNone, Call(Expr("foo")));
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(ty),
                                          ast::StorageClass::kFunction);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_call_expression_cases[] = {
    Params{ast_bool, sem_bool},
    Params{ast_i32, sem_i32},
    Params{ast_u32, sem_u32},
    Params{ast_f32, sem_f32},
    Params{ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{ast_mat3x3<i32>, sem_mat3x3<sem_i32>},
    Params{ast_mat3x3<u32>, sem_mat3x3<sem_u32>},
    Params{ast_mat3x3<f32>, sem_mat3x3<sem_f32>},
    Params{ast_alias<ast_bool>, sem_bool},
    Params{ast_alias<ast_i32>, sem_i32},
    Params{ast_alias<ast_u32>, sem_u32},
    Params{ast_alias<ast_f32>, sem_f32},
    Params{ast_alias<ast_vec3<i32>>, sem_vec3<sem_i32>},
    Params{ast_alias<ast_vec3<u32>>, sem_vec3<sem_u32>},
    Params{ast_alias<ast_vec3<f32>>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_mat3x3<i32>>, sem_mat3x3<sem_i32>},
    Params{ast_alias<ast_mat3x3<u32>>, sem_mat3x3<sem_u32>},
    Params{ast_alias<ast_mat3x3<f32>>, sem_mat3x3<sem_f32>},
};
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         InferTypeTest_FromCallExpression,
                         testing::ValuesIn(from_call_expression_cases));

}  // namespace InferTypeTest

}  // namespace
}  // namespace resolver
}  // namespace tint
