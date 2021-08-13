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
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
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
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
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

    ParamsFor<i32, bool>(),  //
    ParamsFor<i32, u32>(),   //
    ParamsFor<i32, f32>(),   //

    ParamsFor<u32, bool>(),  //
    ParamsFor<u32, i32>(),   //
    ParamsFor<u32, f32>(),   //

    ParamsFor<f32, bool>(),  //
    ParamsFor<f32, u32>(),   //
    ParamsFor<f32, i32>(),   //

    ParamsFor<vec3<bool>, vec3<u32>>(),  //
    ParamsFor<vec3<bool>, vec3<i32>>(),  //
    ParamsFor<vec3<bool>, vec3<f32>>(),  //

    ParamsFor<vec3<i32>, vec3<bool>>(),  //
    ParamsFor<vec3<i32>, vec3<u32>>(),   //
    ParamsFor<vec3<i32>, vec3<f32>>(),   //

    ParamsFor<vec3<u32>, vec3<bool>>(),  //
    ParamsFor<vec3<u32>, vec3<i32>>(),   //
    ParamsFor<vec3<u32>, vec3<f32>>(),   //

    ParamsFor<vec3<f32>, vec3<bool>>(),  //
    ParamsFor<vec3<f32>, vec3<u32>>(),   //
    ParamsFor<vec3<f32>, vec3<i32>>(),   //
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
  auto* a =
      Var("a", ty.f32(), ast::StorageClass::kNone,
          Construct(Source{{12, 34}}, ty.f32(), Construct(ty.array<f32, 4>())));
  WrapInFunction(a);

  ASSERT_FALSE(r()->Resolve());
  ASSERT_EQ(r()->error(),
            "12:34 error: cannot construct 'f32' with a value of type "
            "'array<f32, 4>'");
}

}  // namespace ConversionConstructorTest

namespace ArrayConstructor {

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ZeroValue_Pass) {
  // array<u32, 10>();
  auto* tc = array<u32, 10>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_type_match) {
  // array<u32, 3>(0u, 10u. 20u);
  auto* tc =
      array<u32, 3>(create<ast::ScalarConstructorExpression>(Literal(0u)),
                    create<ast::ScalarConstructorExpression>(Literal(10u)),
                    create<ast::ScalarConstructorExpression>(Literal(20u)));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_type_Mismatch_U32F32) {
  // array<u32, 3>(0u, 1.0f, 20u);
  auto* tc = array<u32, 3>(
      create<ast::ScalarConstructorExpression>(Literal(0u)),
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Literal(20u)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'u32', found 'f32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_F32I32) {
  // array<f32, 1>(1);
  auto* tc = array<f32, 1>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_U32I32) {
  // array<u32, 6>(1, 0u, 0u, 0u, 0u, 0u);
  auto* tc = array<u32, 1>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      create<ast::ScalarConstructorExpression>(Literal(0u)),
      create<ast::ScalarConstructorExpression>(Literal(0u)),
      create<ast::ScalarConstructorExpression>(Literal(0u)),
      create<ast::ScalarConstructorExpression>(Literal(0u)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_Vec2) {
  // array<i32, 3>(1, vec2<i32>());
  auto* tc = array<i32, 3>(create<ast::ScalarConstructorExpression>(Literal(1)),
                           create<ast::TypeConstructorExpression>(
                               Source{{12, 34}}, ty.vec2<i32>(), ExprList()));
  WrapInFunction(tc);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'i32', found 'vec2<i32>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfVector_SubElemTypeMismatch_I32U32) {
  // array<vec3<i32>, 2>(vec3<i32>(), vec3<u32>());
  auto* e0 = vec3<i32>();
  SetSource(Source::Location({12, 34}));
  auto* e1 = vec3<u32>();
  auto* t = Construct(ty.array(ty.vec3<i32>(), 2), e0, e1);
  WrapInFunction(t);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'vec3<i32>', found 'vec3<u32>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfVector_SubElemTypeMismatch_I32Bool) {
  // array<vec3<i32>, 2>(vec3<i32>(), vec3<bool>(true, true, false));
  SetSource(Source::Location({12, 34}));
  auto* e0 = vec3<bool>(true, true, false);
  auto* e1 = vec3<i32>();
  auto* t = Construct(ty.array(ty.vec3<i32>(), 2), e0, e1);
  WrapInFunction(t);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'vec3<i32>', found 'vec3<bool>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfArray_SubElemSizeMismatch) {
  // array<array<i32, 2>, 2>(array<i32, 3>(), array<i32, 2>());
  SetSource(Source::Location({12, 34}));
  auto* e0 = array<i32, 3>();
  auto* e1 = array<i32, 2>();
  auto* t = Construct(ty.array(ty.array<i32, 2>(), 2), e0, e1);
  WrapInFunction(t);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'array<i32, 2>', found 'array<i32, 3>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfArray_SubElemTypeMismatch) {
  // array<array<i32, 2>, 2>(array<i32, 2>(), array<u32, 2>());
  auto* e0 = array<i32, 2>();
  SetSource(Source::Location({12, 34}));
  auto* e1 = array<u32, 2>();
  auto* t = Construct(ty.array(ty.array<i32, 2>(), 2), e0, e1);
  WrapInFunction(t);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in array constructor does not match array type: "
            "expected 'array<i32, 2>', found 'array<u32, 2>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_TooFewElements) {
  // array<i32, 4>(1, 2, 3);
  SetSource(Source::Location({12, 34}));
  auto* tc =
      array<i32, 4>(create<ast::ScalarConstructorExpression>(Literal(1)),
                    create<ast::ScalarConstructorExpression>(Literal(2)),
                    create<ast::ScalarConstructorExpression>(Literal(3)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array constructor has too few elements: expected 4, "
            "found 3");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_TooManyElements) {
  // array<i32, 4>(1, 2, 3, 4, 5);
  SetSource(Source::Location({12, 34}));
  auto* tc =
      array<i32, 4>(create<ast::ScalarConstructorExpression>(Literal(1)),
                    create<ast::ScalarConstructorExpression>(Literal(2)),
                    create<ast::ScalarConstructorExpression>(Literal(3)),
                    create<ast::ScalarConstructorExpression>(Literal(4)),
                    create<ast::ScalarConstructorExpression>(Literal(5)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array constructor has too many "
            "elements: expected 4, "
            "found 5");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_Runtime) {
  // array<f32>(1);
  auto* tc = array<i32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "error: cannot init a runtime-sized array");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_RuntimeZeroValue) {
  // array<f32>();
  auto* tc = array<i32>();
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "error: cannot init a runtime-sized array");
}

}  // namespace ArrayConstructor

namespace VectorConstructor {

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<u32>(1u, create<ast::ScalarConstructorExpression>(
                               Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<i32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<bool>(true, create<ast::ScalarConstructorExpression>(
                                  Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_Vec3ArgumentCardinalityTooLarge) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_Vec4ArgumentCardinalityTooLarge) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec4<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsScalar) {
  auto* tc = vec2<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsVector) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsVectorAndScalar) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_InvalidArgumentType) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Success_ZeroValue) {
  auto* tc = vec2<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2F32_Success_Scalar) {
  auto* tc = vec2<f32>(1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2U32_Success_Scalar) {
  auto* tc = vec2<u32>(1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2I32_Success_Scalar) {
  auto* tc = vec2<i32>(1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2Bool_Success_Scalar) {
  auto* tc = vec2<bool>(true, false);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Success_Identity) {
  auto* tc = vec2<f32>(vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Success_Vec2TypeConversion) {
  auto* tc = vec2<f32>(vec2<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<f32>(
      1.0f, 1.0f,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<u32>(
      1u,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1u);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<i32>(
      1,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<bool>(
      true,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      false);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_Vec4ArgumentCardinalityTooLarge) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec4<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooFewArgumentsScalar) {
  auto* tc = vec3<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsScalar) {
  auto* tc = vec3<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooFewArgumentsVec2) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec2) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec2AndScalar) {
  auto* tc = vec3<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec2<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec3) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_InvalidArgumentType) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Success_ZeroValue) {
  auto* tc = vec3<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3F32_Success_Scalar) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3U32_Success_Scalar) {
  auto* tc = vec3<u32>(1u, 1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3I32_Success_Scalar) {
  auto* tc = vec3<i32>(1, 1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3Bool_Success_Scalar) {
  auto* tc = vec3<bool>(true, false, true);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Success_Vec2AndScalar) {
  auto* tc = vec3<f32>(vec2<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Success_ScalarAndVec2) {
  auto* tc = vec3<f32>(1.0f, vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Success_Identity) {
  auto* tc = vec3<f32>(vec3<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Success_Vec3TypeConversion) {
  auto* tc = vec3<f32>(vec3<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<f32>(
      1.0f, 1.0f,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<u32>(
      1u, 1u,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1u);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<i32>(
      1, 1,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<bool>(
      true, false,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      true);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsScalar) {
  auto* tc = vec4<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsScalar) {
  auto* tc = vec4<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 58}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsVec2AndScalar) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndScalars) {
  auto* tc = vec4<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec2<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Scalar) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 46}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Vec2) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndScalars) {
  auto* tc = vec4<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec3<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec2) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_InvalidArgumentType) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_ZeroValue) {
  auto* tc = vec4<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4F32_Success_Scalar) {
  auto* tc = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4U32_Success_Scalar) {
  auto* tc = vec4<u32>(1u, 1u, 1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4I32_Success_Scalar) {
  auto* tc = vec4<i32>(1, 1, 1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4Bool_Success_Scalar) {
  auto* tc = vec4<bool>(true, false, true, false);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_Vec2ScalarScalar) {
  auto* tc = vec4<f32>(vec2<f32>(), 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_ScalarVec2Scalar) {
  auto* tc = vec4<f32>(1.0f, vec2<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_ScalarScalarVec2) {
  auto* tc = vec4<f32>(1.0f, 1.0f, vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_Vec2AndVec2) {
  auto* tc = vec4<f32>(vec2<f32>(), vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_Vec3AndScalar) {
  auto* tc = vec4<f32>(vec3<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_ScalarAndVec3) {
  auto* tc = vec4<f32>(1.0f, vec3<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_Identity) {
  auto* tc = vec4<f32>(vec4<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Success_Vec4TypeConversion) {
  auto* tc = vec4<f32>(vec4<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_NestedVectorConstructors_InnerError) {
  auto* tc = vec4<f32>(vec4<f32>(1.0f, 1.0f,
                                 vec3<f32>(Expr(Source{{12, 34}}, 1.0f),
                                           Expr(Source{{12, 34}}, 1.0f))),
                       1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_NestedVectorConstructors_Success) {
  auto* tc = vec4<f32>(vec3<f32>(vec2<f32>(1.0f, 1.0f), 1.0f), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_Alias_Argument_Error) {
  auto* alias = Alias("UnsignedInt", ty.u32());
  Global("uint_var", ty.Of(alias), ast::StorageClass::kPrivate);

  auto* tc = vec2<f32>(Expr(Source{{12, 34}}, "uint_var"));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'UnsignedInt'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_Alias_Argument_Success) {
  auto* f32_alias = Alias("Float32", ty.f32());
  auto* vec2_alias = Alias("VectorFloat2", ty.vec2<f32>());
  Global("my_f32", ty.Of(f32_alias), ast::StorageClass::kPrivate);
  Global("my_vec2", ty.Of(vec2_alias), ast::StorageClass::kPrivate);

  auto* tc = vec3<f32>("my_vec2", "my_f32");
  WrapInFunction(tc);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_ElementTypeAlias_Error) {
  auto* f32_alias = Alias("Float32", ty.f32());

  // vec2<Float32>(1.0f, 1u)
  auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
  auto* tc = create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, vec_type,
      ExprList(1.0f, create<ast::ScalarConstructorExpression>(Source{{12, 40}},
                                                              Literal(1u))));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:40 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_ElementTypeAlias_Success) {
  auto* f32_alias = Alias("Float32", ty.f32());

  // vec2<Float32>(1.0f, 1.0f)
  auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, vec_type,
                                                    ExprList(1.0f, 1.0f));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_ArgumentElementTypeAlias_Error) {
  auto* f32_alias = Alias("Float32", ty.f32());

  // vec3<u32>(vec<Float32>(), 1.0f)
  auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
  auto* tc = vec3<u32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, vec_type, ExprList()),
                       1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'f32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_ArgumentElementTypeAlias_Success) {
  auto* f32_alias = Alias("Float32", ty.f32());

  // vec3<f32>(vec<Float32>(), 1.0f)
  auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, vec_type, ExprList()),
                       1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace VectorConstructor

namespace MatrixConstructor {
struct MatrixDimensions {
  uint32_t rows;
  uint32_t columns;
};

static std::string MatrixStr(const MatrixDimensions& dimensions,
                             std::string subtype = "f32") {
  return "mat" + std::to_string(dimensions.columns) + "x" +
         std::to_string(dimensions.rows) + "<" + subtype + ">";
}

static std::string VecStr(uint32_t dimensions, std::string subtype = "f32") {
  return "vec" + std::to_string(dimensions) + "<" + subtype + ">";
}

using MatrixConstructorTest = ResolverTestWithParam<MatrixDimensions>;

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_TooFewArguments) {
  // matNxM<f32>(vecM<f32>(), ...); with N - 1 arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected " + std::to_string(param.columns) + " '" +
                VecStr(param.rows) + "' arguments in '" + MatrixStr(param) +
                "' constructor, found " + std::to_string(param.columns - 1));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_TooManyArguments) {
  // matNxM<f32>(vecM<f32>(), ...); with N + 1 arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns + 1; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected " + std::to_string(param.columns) + " '" +
                VecStr(param.rows) + "' arguments in '" + MatrixStr(param) +
                "' constructor, found " + std::to_string(param.columns + 1));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_InvalidArgumentType) {
  // matNxM<f32>(1.0, 1.0, ...); N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    args.push_back(create<ast::ScalarConstructorExpression>(Source{{12, i}},
                                                            Literal(1.0f)));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found 'f32'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_TooFewRowsInVectorArgument) {
  // matNxM<f32>(vecM<f32>(),...,vecM-1<f32>());

  const auto param = GetParam();

  // Skip the test if parameters would have resulted in an invalid vec1 type.
  if (param.rows == 2) {
    return;
  }

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* valid_vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, valid_vec_type, ExprList()));
  }
  const size_t kInvalidLoc = 2 * (param.columns - 1);
  auto* invalid_vec_type = ty.vec<f32>(param.rows - 1);
  args.push_back(create<ast::TypeConstructorExpression>(
      Source{{12, kInvalidLoc}}, invalid_vec_type, ExprList()));

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:" + std::to_string(kInvalidLoc) +
                              " error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows - 1) + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_TooManyRowsInVectorArgument) {
  // matNxM<f32>(vecM<f32>(),...,vecM+1<f32>());

  const auto param = GetParam();

  // Skip the test if parameters would have resuled in an invalid vec5 type.
  if (param.rows == 4) {
    return;
  }

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* valid_vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, valid_vec_type, ExprList()));
  }
  const size_t kInvalidLoc = 2 * (param.columns - 1);
  auto* invalid_vec_type = ty.vec<f32>(param.rows + 1);
  args.push_back(create<ast::TypeConstructorExpression>(
      Source{{12, kInvalidLoc}}, invalid_vec_type, ExprList()));

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:" + std::to_string(kInvalidLoc) +
                              " error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows + 1) + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_ArgumentVectorElementTypeMismatch) {
  // matNxM<f32>(vecM<u32>(), ...); with N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<u32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows, "u32") + "'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ZeroValue_Success) {
  // matNxM<f32>();

  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 40}},
                                                    matrix_type, ExprList());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_WithArguments_Success) {
  // matNxM<f32>(vecM<f32>(), ...); with N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Error) {
  // matNxM<Float32>(vecM<u32>(), ...); with N arguments

  const auto param = GetParam();
  auto* f32_alias = Alias("Float32", ty.f32());

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(ty.u32(), param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat(ty.Of(f32_alias), param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected argument type '" + VecStr(param.rows) +
                "' in '" + MatrixStr(param, "Float32") +
                "' constructor, found '" + VecStr(param.rows, "u32") + "'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Success) {
  // matNxM<Float32>(vecM<f32>(), ...); with N arguments

  const auto param = GetParam();
  auto* f32_alias = Alias("Float32", ty.f32());

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat(ty.Of(f32_alias), param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_MatrixConstructor_ArgumentTypeAlias_Error) {
  auto* alias = Alias("VectorUnsigned2", ty.vec2<u32>());
  auto* tc = mat2x2<f32>(create<ast::TypeConstructorExpression>(
                             Source{{12, 34}}, ty.Of(alias), ExprList()),
                         vec2<f32>());
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected argument type 'vec2<f32>' in 'mat2x2<f32>' "
            "constructor, found 'VectorUnsigned2'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentTypeAlias_Success) {
  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* vec_type = ty.vec<f32>(param.rows);
  auto* vec_alias = Alias("VectorFloat2", vec_type);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, ty.Of(vec_alias), ExprList()));
  }

  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentElementTypeAlias_Error) {
  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* f32_alias = Alias("UnsignedInt", ty.u32());

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(ty.Of(f32_alias), param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows, "UnsignedInt") + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_ArgumentElementTypeAlias_Success) {
  const auto param = GetParam();
  auto* f32_alias = Alias("Float32", ty.f32());

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(ty.Of(f32_alias), param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         MatrixConstructorTest,
                         testing::Values(MatrixDimensions{2, 2},
                                         MatrixDimensions{3, 2},
                                         MatrixDimensions{4, 2},
                                         MatrixDimensions{2, 3},
                                         MatrixDimensions{3, 3},
                                         MatrixDimensions{4, 3},
                                         MatrixDimensions{2, 4},
                                         MatrixDimensions{3, 4},
                                         MatrixDimensions{4, 4}));
}  // namespace MatrixConstructor

namespace StructConstructor {
using builder::CreatePtrs;
using builder::CreatePtrsFor;
using builder::f32;
using builder::i32;
using builder::mat2x2;
using builder::mat3x3;
using builder::mat4x4;
using builder::u32;
using builder::vec2;
using builder::vec3;
using builder::vec4;

constexpr CreatePtrs all_types[] = {
    CreatePtrsFor<bool>(),         //
    CreatePtrsFor<u32>(),          //
    CreatePtrsFor<i32>(),          //
    CreatePtrsFor<f32>(),          //
    CreatePtrsFor<vec4<bool>>(),   //
    CreatePtrsFor<vec2<i32>>(),    //
    CreatePtrsFor<vec3<u32>>(),    //
    CreatePtrsFor<vec4<f32>>(),    //
    CreatePtrsFor<mat2x2<f32>>(),  //
    CreatePtrsFor<mat3x3<f32>>(),  //
    CreatePtrsFor<mat4x4<f32>>()   //
};

auto number_of_members = testing::Values(2u, 32u, 64u);

using StructConstructorInputsTest =
    ResolverTestWithParam<std::tuple<CreatePtrs,  // struct member type
                                     uint32_t>>;  // number of struct members
TEST_P(StructConstructorInputsTest, TooFew) {
  auto& param = GetParam();
  auto& str_params = std::get<0>(param);
  uint32_t N = std::get<1>(param);

  ast::StructMemberList members;
  ast::ExpressionList values;
  for (uint32_t i = 0; i < N; i++) {
    auto* struct_type = str_params.ast(*this);
    members.push_back(Member("member_" + std::to_string(i), struct_type));
    if (i < N - 1) {
      auto* ctor_value_expr = str_params.expr(*this, 0);
      values.push_back(ctor_value_expr);
    }
  }
  auto* s = Structure("s", members);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.Of(s),
                                                    values);
  WrapInFunction(tc);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: struct constructor has too few inputs: expected " +
                std::to_string(N) + ", found " + std::to_string(N - 1));
}

TEST_P(StructConstructorInputsTest, TooMany) {
  auto& param = GetParam();
  auto& str_params = std::get<0>(param);
  uint32_t N = std::get<1>(param);

  ast::StructMemberList members;
  ast::ExpressionList values;
  for (uint32_t i = 0; i < N + 1; i++) {
    if (i < N) {
      auto* struct_type = str_params.ast(*this);
      members.push_back(Member("member_" + std::to_string(i), struct_type));
    }
    auto* ctor_value_expr = str_params.expr(*this, 0);
    values.push_back(ctor_value_expr);
  }
  auto* s = Structure("s", members);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.Of(s),
                                                    values);
  WrapInFunction(tc);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: struct constructor has too many inputs: expected " +
                std::to_string(N) + ", found " + std::to_string(N + 1));
}

INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         StructConstructorInputsTest,
                         testing::Combine(testing::ValuesIn(all_types),
                                          number_of_members));
using StructConstructorTypeTest =
    ResolverTestWithParam<std::tuple<CreatePtrs,  // struct member type
                                     CreatePtrs,  // constructor value type
                                     uint32_t>>;  // number of struct members
TEST_P(StructConstructorTypeTest, AllTypes) {
  auto& param = GetParam();
  auto& str_params = std::get<0>(param);
  auto& ctor_params = std::get<1>(param);
  uint32_t N = std::get<2>(param);

  if (str_params.ast == ctor_params.ast) {
    return;
  }

  ast::StructMemberList members;
  ast::ExpressionList values;
  // make the last value of the constructor to have a different type
  uint32_t constructor_value_with_different_type = N - 1;
  for (uint32_t i = 0; i < N; i++) {
    auto* struct_type = str_params.ast(*this);
    members.push_back(Member("member_" + std::to_string(i), struct_type));
    auto* ctor_value_expr = (i == constructor_value_with_different_type)
                                ? ctor_params.expr(*this, 0)
                                : str_params.expr(*this, 0);
    values.push_back(ctor_value_expr);
  }
  auto* s = Structure("s", members);
  auto* tc = create<ast::TypeConstructorExpression>(ty.Of(s), values);
  WrapInFunction(tc);

  std::string found = FriendlyName(ctor_params.ast(*this));
  std::string expected = FriendlyName(str_params.ast(*this));
  std::stringstream err;
  err << "error: type in struct constructor does not match struct member ";
  err << "type: expected '" << expected << "', found '" << found << "'";
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), err.str());
}

INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         StructConstructorTypeTest,
                         testing::Combine(testing::ValuesIn(all_types),
                                          testing::ValuesIn(all_types),
                                          number_of_members));

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Struct_Nested) {
  auto* inner_m = Member("m", ty.i32());
  auto* inner_s = Structure("inner_s", {inner_m});

  auto* m0 = Member("m0", ty.i32());
  auto* m1 = Member("m1", ty.Of(inner_s));
  auto* m2 = Member("m2", ty.i32());
  auto* s = Structure("s", {m0, m1, m2});

  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.Of(s),
                                                    ExprList(1, 1, 1));
  WrapInFunction(tc);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: type in struct constructor does not match struct member "
            "type: expected 'inner_s', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Struct) {
  auto* m = Member("m", ty.i32());
  auto* s = Structure("MyInputs", {m});
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.Of(s),
                                                    ExprList());
  WrapInFunction(tc);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Struct_Empty) {
  auto* str = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                                 Member("c", ty.vec3<i32>()),
                             });

  WrapInFunction(Construct(ty.Of(str)));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace StructConstructor

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_Atomic) {
  WrapInFunction(
      Call("ignore", Construct(Source{{12, 34}}, ty.atomic(ty.i32()))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

TEST_F(ResolverTypeConstructorValidationTest,
       NonConstructibleType_AtomicArray) {
  WrapInFunction(Call(
      "ignore", Construct(ty.array(ty.atomic(Source{{12, 34}}, ty.i32()), 4))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: array constructor has non-constructible element type");
}

TEST_F(ResolverTypeConstructorValidationTest,
       NonConstructibleType_AtomicStructMember) {
  auto* str = Structure("S", {Member("a", ty.atomic(ty.i32()))});
  WrapInFunction(Call("ignore", Construct(Source{{12, 34}}, ty.Of(str))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: struct constructor has non-constructible type");
}

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_Sampler) {
  WrapInFunction(Call(
      "ignore",
      Construct(Source{{12, 34}}, ty.sampler(ast::SamplerKind::kSampler))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
