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
using builder::alias;
using builder::alias1;
using builder::alias2;
using builder::alias3;
using builder::CreatePtrs;
using builder::CreatePtrsFor;
using builder::DataType;
using builder::f32;
using builder::i32;
using builder::mat2x2;
using builder::mat2x3;
using builder::mat3x2;
using builder::mat3x3;
using builder::mat4x4;
using builder::u32;
using builder::vec2;
using builder::vec3;
using builder::vec4;

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

namespace ConversionConstructorTest {
struct Params {
  builder::ast_type_func_ptr lhs_type;
  builder::ast_type_func_ptr rhs_type;
  builder::ast_expr_func_ptr rhs_value_expr;
};

template <typename LhsType, typename RhsType>
constexpr Params ParamsFor() {
  return Params{DataType<LhsType>::AST, DataType<RhsType>::AST,
                DataType<RhsType>::Expr};
}

static constexpr Params valid_cases[] = {
    // Direct init (non-conversions)
    ParamsFor<bool, bool>(),              //
    ParamsFor<i32, i32>(),                //
    ParamsFor<u32, u32>(),                //
    ParamsFor<f32, f32>(),                //
    ParamsFor<vec3<bool>, vec3<bool>>(),  //
    ParamsFor<vec3<i32>, vec3<i32>>(),    //
    ParamsFor<vec3<u32>, vec3<u32>>(),    //
    ParamsFor<vec3<f32>, vec3<f32>>(),    //

    // Splat
    ParamsFor<vec3<bool>, bool>(),  //
    ParamsFor<vec3<i32>, i32>(),    //
    ParamsFor<vec3<u32>, u32>(),    //
    ParamsFor<vec3<f32>, f32>(),    //

    // Conversion
    ParamsFor<bool, u32>(),  //
    ParamsFor<bool, i32>(),  //
    ParamsFor<bool, f32>(),  //

    ParamsFor<i32, u32>(),  //
    ParamsFor<i32, f32>(),  //

    ParamsFor<u32, i32>(),  //
    ParamsFor<u32, f32>(),  //

    ParamsFor<f32, u32>(),  //
    ParamsFor<f32, i32>(),  //

    ParamsFor<vec3<bool>, vec3<u32>>(),  //
    ParamsFor<vec3<bool>, vec3<i32>>(),  //
    ParamsFor<vec3<bool>, vec3<f32>>(),  //

    ParamsFor<vec3<i32>, vec3<u32>>(),  //
    ParamsFor<vec3<i32>, vec3<f32>>(),  //

    ParamsFor<vec3<u32>, vec3<i32>>(),  //
    ParamsFor<vec3<u32>, vec3<f32>>(),  //

    ParamsFor<vec3<f32>, vec3<u32>>(),  //
    ParamsFor<vec3<f32>, vec3<i32>>(),  //
};

using ConversionConstructorValidTest = ResolverTestWithParam<Params>;
TEST_P(ConversionConstructorValidTest, All) {
  auto& params = GetParam();

  // var a : <lhs_type1> = <lhs_type2>(<rhs_type>(<rhs_value_expr>));
  auto* lhs_type1 = params.lhs_type(*this);
  auto* lhs_type2 = params.lhs_type(*this);
  auto* rhs_type = params.rhs_type(*this);
  auto* rhs_value_expr = params.rhs_value_expr(*this, 0);

  std::stringstream ss;
  ss << FriendlyName(lhs_type1) << " = " << FriendlyName(lhs_type2) << "("
     << FriendlyName(rhs_type) << "(<rhs value expr>))";
  SCOPED_TRACE(ss.str());

  auto* a = Var("a", lhs_type1, ast::StorageClass::kNone,
                Construct(lhs_type2, Construct(rhs_type, rhs_value_expr)));

  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         ConversionConstructorValidTest,
                         testing::ValuesIn(valid_cases));

constexpr CreatePtrs all_types[] = {
    CreatePtrsFor<bool>(),         //
    CreatePtrsFor<u32>(),          //
    CreatePtrsFor<i32>(),          //
    CreatePtrsFor<f32>(),          //
    CreatePtrsFor<vec3<bool>>(),   //
    CreatePtrsFor<vec3<i32>>(),    //
    CreatePtrsFor<vec3<u32>>(),    //
    CreatePtrsFor<vec3<f32>>(),    //
    CreatePtrsFor<mat3x3<i32>>(),  //
    CreatePtrsFor<mat3x3<u32>>(),  //
    CreatePtrsFor<mat3x3<f32>>(),  //
    CreatePtrsFor<mat2x3<i32>>(),  //
    CreatePtrsFor<mat2x3<u32>>(),  //
    CreatePtrsFor<mat2x3<f32>>(),  //
    CreatePtrsFor<mat3x2<i32>>(),  //
    CreatePtrsFor<mat3x2<u32>>(),  //
    CreatePtrsFor<mat3x2<f32>>()   //
};

using ConversionConstructorInvalidTest =
    ResolverTestWithParam<std::tuple<CreatePtrs,  // lhs
                                     CreatePtrs   // rhs
                                     >>;
TEST_P(ConversionConstructorInvalidTest, All) {
  auto& params = GetParam();

  auto& lhs_params = std::get<0>(params);
  auto& rhs_params = std::get<1>(params);

  // Skip test for valid cases
  for (auto& v : valid_cases) {
    if (v.lhs_type == lhs_params.ast && v.rhs_type == rhs_params.ast &&
        v.rhs_value_expr == rhs_params.expr) {
      return;
    }
  }
  // Skip non-conversions
  if (lhs_params.ast == rhs_params.ast) {
    return;
  }

  // var a : <lhs_type1> = <lhs_type2>(<rhs_type>(<rhs_value_expr>));
  auto* lhs_type1 = lhs_params.ast(*this);
  auto* lhs_type2 = lhs_params.ast(*this);
  auto* rhs_type = rhs_params.ast(*this);
  auto* rhs_value_expr = rhs_params.expr(*this, 0);

  std::stringstream ss;
  ss << FriendlyName(lhs_type1) << " = " << FriendlyName(lhs_type2) << "("
     << FriendlyName(rhs_type) << "(<rhs value expr>))";
  SCOPED_TRACE(ss.str());

  auto* a = Var("a", lhs_type1, ast::StorageClass::kNone,
                Construct(lhs_type2, Construct(rhs_type, rhs_value_expr)));

  // Self-assign 'a' to force the expression to be resolved so we can test its
  // type below
  auto* a_ident = Expr("a");
  WrapInFunction(Decl(a), Assign(a_ident, "a"));

  ASSERT_FALSE(r()->Resolve());
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         ConversionConstructorInvalidTest,
                         testing::Combine(testing::ValuesIn(all_types),
                                          testing::ValuesIn(all_types)));

TEST_F(ResolverTypeConstructorValidationTest,
       ConversionConstructorInvalid_TooManyInitializers) {
  auto* a = Var("a", ty.f32(), ast::StorageClass::kNone,
                Construct(Source{{12, 34}}, ty.f32(), Expr(1.0f), Expr(2.0f)));
  WrapInFunction(a);

  ASSERT_FALSE(r()->Resolve());
  ASSERT_EQ(r()->error(),
            "12:34 error: expected zero or one value in constructor, got 2");
}

TEST_F(ResolverTypeConstructorValidationTest,
       ConversionConstructorInvalid_InvalidInitializer) {
  auto* a = Var("a", ty.f32(), ast::StorageClass::kNone,
                Construct(Source{{12, 34}}, ty.f32(), Expr(true)));
  WrapInFunction(a);

  ASSERT_FALSE(r()->Resolve());
  ASSERT_EQ(r()->error(),
            "12:34 error: cannot construct 'f32' with a value of type 'bool'");
}

}  // namespace ConversionConstructorTest

}  // namespace
}  // namespace resolver
}  // namespace tint
