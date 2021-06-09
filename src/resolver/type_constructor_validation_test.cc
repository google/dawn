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

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T>
using alias = builder::alias<T>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;

class ResolverTypeConstructorValidationTest : public resolver::TestHelper,
                                              public testing::Test {};

namespace InferTypeTest {
struct Params {
  builder::ast_type_func_ptr create_rhs_ast_type;
  builder::ast_expr_func_ptr create_rhs_ast_value;
  builder::sem_type_func_ptr create_rhs_sem_type;
};

template <typename T>
constexpr Params ParamsFor() {
  return Params{DataType<T>::AST, DataType<T>::Expr, DataType<T>::Sem};
}

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

  auto* constructor_expr = params.create_rhs_ast_value(*this, 0);

  auto* a = Var("a", nullptr, ast::StorageClass::kNone, constructor_expr);
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(*this),
                                          ast::StorageClass::kFunction,
                                          ast::Access::kReadWrite);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}

static constexpr Params from_constructor_expression_cases[] = {
    ParamsFor<bool>(),
    ParamsFor<i32>(),
    ParamsFor<u32>(),
    ParamsFor<f32>(),
    ParamsFor<vec3<i32>>(),
    ParamsFor<vec3<u32>>(),
    ParamsFor<vec3<f32>>(),
    ParamsFor<mat3x3<i32>>(),
    ParamsFor<mat3x3<u32>>(),
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias<mat3x3<i32>>>(),
    ParamsFor<alias<mat3x3<u32>>>(),
    ParamsFor<alias<mat3x3<f32>>>(),
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

  auto* arith_lhs_expr = params.create_rhs_ast_value(*this, 2);
  auto* arith_rhs_expr = params.create_rhs_ast_value(*this, 3);
  auto* constructor_expr = Mul(arith_lhs_expr, arith_rhs_expr);

  auto* a = Var("a", nullptr, constructor_expr);
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(*this),
                                          ast::StorageClass::kFunction,
                                          ast::Access::kReadWrite);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_arithmetic_expression_cases[] = {
    ParamsFor<i32>(),       ParamsFor<u32>(),         ParamsFor<f32>(),
    ParamsFor<vec3<f32>>(), ParamsFor<mat3x3<f32>>(),

    // TODO(amaiorano): Uncomment once https://crbug.com/tint/680 is fixed
    // ParamsFor<alias<ty_i32>>(),
    // ParamsFor<alias<ty_u32>>(),
    // ParamsFor<alias<ty_f32>>(),
    // ParamsFor<alias<ty_vec3<f32>>>(),
    // ParamsFor<alias<ty_mat3x3<f32>>>(),
};
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         InferTypeTest_FromArithmeticExpression,
                         testing::ValuesIn(from_arithmetic_expression_cases));

using InferTypeTest_FromCallExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromCallExpression, All) {
  // e.g. for vec3<f32>
  //
  // fn foo() -> vec3<f32> {
  //   return vec3<f32>();
  // }
  //
  // fn bar()
  // {
  //   var a = foo();
  // }
  auto& params = GetParam();

  Func("foo", {}, params.create_rhs_ast_type(*this),
       {Return(Construct(params.create_rhs_ast_type(*this)))}, {});

  auto* a = Var("a", nullptr, Call("foo"));
  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  auto* got = TypeOf(a_ident);
  auto* expected = create<sem::Reference>(params.create_rhs_sem_type(*this),
                                          ast::StorageClass::kFunction,
                                          ast::Access::kReadWrite);
  ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_call_expression_cases[] = {
    ParamsFor<bool>(),
    ParamsFor<i32>(),
    ParamsFor<u32>(),
    ParamsFor<f32>(),
    ParamsFor<vec3<i32>>(),
    ParamsFor<vec3<u32>>(),
    ParamsFor<vec3<f32>>(),
    ParamsFor<mat3x3<i32>>(),
    ParamsFor<mat3x3<u32>>(),
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias<mat3x3<i32>>>(),
    ParamsFor<alias<mat3x3<u32>>>(),
    ParamsFor<alias<mat3x3<f32>>>(),
};
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         InferTypeTest_FromCallExpression,
                         testing::ValuesIn(from_call_expression_cases));

}  // namespace InferTypeTest

}  // namespace
}  // namespace resolver
}  // namespace tint
