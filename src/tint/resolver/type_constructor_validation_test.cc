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

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ::testing::HasSubstr;

// Helpers and typedefs
using builder::alias;
using builder::alias1;
using builder::alias2;
using builder::alias3;
using builder::CreatePtrs;
using builder::CreatePtrsFor;
using builder::DataType;
using builder::mat2x2;
using builder::mat2x3;
using builder::mat3x2;
using builder::mat3x3;
using builder::mat4x4;
using builder::vec2;
using builder::vec3;
using builder::vec4;

class ResolverTypeConstructorValidationTest : public resolver::TestHelper, public testing::Test {};

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
    auto* a = Var("a", nullptr, ast::StorageClass::kNone, Expr(1_i));
    auto* b = Var("b", nullptr, ast::StorageClass::kNone, Expr("a"));
    auto* a_ident = Expr("a");
    auto* b_ident = Expr("b");

    WrapInFunction(a, b, Assign(a_ident, "a"), Assign(b_ident, "b"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(a_ident)->Is<sem::Reference>());
    EXPECT_TRUE(TypeOf(a_ident)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_EQ(TypeOf(a_ident)->As<sem::Reference>()->StorageClass(), ast::StorageClass::kFunction);
    ASSERT_TRUE(TypeOf(b_ident)->Is<sem::Reference>());
    EXPECT_TRUE(TypeOf(b_ident)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_EQ(TypeOf(b_ident)->As<sem::Reference>()->StorageClass(), ast::StorageClass::kFunction);
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
                                            ast::StorageClass::kFunction, ast::Access::kReadWrite);
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
                                            ast::StorageClass::kFunction, ast::Access::kReadWrite);
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
                                            ast::StorageClass::kFunction, ast::Access::kReadWrite);
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

namespace ConversionConstructTest {
enum class Kind {
    Construct,
    Conversion,
};

struct Params {
    Kind kind;
    builder::ast_type_func_ptr lhs_type;
    builder::ast_type_func_ptr rhs_type;
    builder::ast_expr_func_ptr rhs_value_expr;
};

template <typename LhsType, typename RhsType>
constexpr Params ParamsFor(Kind kind) {
    return Params{kind, DataType<LhsType>::AST, DataType<RhsType>::AST, DataType<RhsType>::Expr};
}

static constexpr Params valid_cases[] = {
    // Direct init (non-conversions)
    ParamsFor<bool, bool>(Kind::Construct),              //
    ParamsFor<i32, i32>(Kind::Construct),                //
    ParamsFor<u32, u32>(Kind::Construct),                //
    ParamsFor<f32, f32>(Kind::Construct),                //
    ParamsFor<vec3<bool>, vec3<bool>>(Kind::Construct),  //
    ParamsFor<vec3<i32>, vec3<i32>>(Kind::Construct),    //
    ParamsFor<vec3<u32>, vec3<u32>>(Kind::Construct),    //
    ParamsFor<vec3<f32>, vec3<f32>>(Kind::Construct),    //

    // Splat
    ParamsFor<vec3<bool>, bool>(Kind::Construct),  //
    ParamsFor<vec3<i32>, i32>(Kind::Construct),    //
    ParamsFor<vec3<u32>, u32>(Kind::Construct),    //
    ParamsFor<vec3<f32>, f32>(Kind::Construct),    //

    // Conversion
    ParamsFor<bool, u32>(Kind::Conversion),  //
    ParamsFor<bool, i32>(Kind::Conversion),  //
    ParamsFor<bool, f32>(Kind::Conversion),  //

    ParamsFor<i32, bool>(Kind::Conversion),  //
    ParamsFor<i32, u32>(Kind::Conversion),   //
    ParamsFor<i32, f32>(Kind::Conversion),   //

    ParamsFor<u32, bool>(Kind::Conversion),  //
    ParamsFor<u32, i32>(Kind::Conversion),   //
    ParamsFor<u32, f32>(Kind::Conversion),   //

    ParamsFor<f32, bool>(Kind::Conversion),  //
    ParamsFor<f32, u32>(Kind::Conversion),   //
    ParamsFor<f32, i32>(Kind::Conversion),   //

    ParamsFor<vec3<bool>, vec3<u32>>(Kind::Conversion),  //
    ParamsFor<vec3<bool>, vec3<i32>>(Kind::Conversion),  //
    ParamsFor<vec3<bool>, vec3<f32>>(Kind::Conversion),  //

    ParamsFor<vec3<i32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<i32>, vec3<u32>>(Kind::Conversion),   //
    ParamsFor<vec3<i32>, vec3<f32>>(Kind::Conversion),   //

    ParamsFor<vec3<u32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<u32>, vec3<i32>>(Kind::Conversion),   //
    ParamsFor<vec3<u32>, vec3<f32>>(Kind::Conversion),   //

    ParamsFor<vec3<f32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<f32>, vec3<u32>>(Kind::Conversion),   //
    ParamsFor<vec3<f32>, vec3<i32>>(Kind::Conversion),   //
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

    auto* arg = Construct(rhs_type, rhs_value_expr);
    auto* tc = Construct(lhs_type2, arg);
    auto* a = Var("a", lhs_type1, ast::StorageClass::kNone, tc);

    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    switch (params.kind) {
        case Kind::Construct: {
            auto* ctor = call->Target()->As<sem::TypeConstructor>();
            ASSERT_NE(ctor, nullptr);
            EXPECT_EQ(call->Type(), ctor->ReturnType());
            ASSERT_EQ(ctor->Parameters().size(), 1u);
            EXPECT_EQ(ctor->Parameters()[0]->Type(), TypeOf(arg));
            break;
        }
        case Kind::Conversion: {
            auto* conv = call->Target()->As<sem::TypeConversion>();
            ASSERT_NE(conv, nullptr);
            EXPECT_EQ(call->Type(), conv->ReturnType());
            ASSERT_EQ(conv->Parameters().size(), 1u);
            EXPECT_EQ(conv->Parameters()[0]->Type(), TypeOf(arg));
            break;
        }
    }
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

using ConversionConstructorInvalidTest = ResolverTestWithParam<std::tuple<CreatePtrs,  // lhs
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

TEST_F(ResolverTypeConstructorValidationTest, ConversionConstructorInvalid_TooManyInitializers) {
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone,
                  Construct(Source{{12, 34}}, ty.f32(), Expr(1.0f), Expr(2.0f)));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    ASSERT_EQ(r()->error(), "12:34 error: expected zero or one value in constructor, got 2");
}

TEST_F(ResolverTypeConstructorValidationTest, ConversionConstructorInvalid_InvalidInitializer) {
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone,
                  Construct(Source{{12, 34}}, ty.f32(), Construct(ty.array<f32, 4>())));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    ASSERT_EQ(r()->error(),
              "12:34 error: cannot construct 'f32' with a value of type "
              "'array<f32, 4>'");
}

}  // namespace ConversionConstructTest

namespace ArrayConstructor {

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_ZeroValue_Pass) {
    // array<u32, 10u>();
    auto* tc = array<u32, 10>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 0u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_type_match) {
    // array<u32, 3u>(0u, 10u. 20u);
    auto* tc = array<u32, 3>(Expr(0_u), Expr(10_u), Expr(20_u));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::U32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_type_Mismatch_U32F32) {
    // array<u32, 3u>(0u, 1.0f, 20u);
    auto* tc = array<u32, 3>(Expr(0_u), Expr(Source{{12, 34}}, 1.0f), Expr(20_u));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'u32', found 'f32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_F32I32) {
    // array<f32, 1u>(1i);
    auto* tc = array<f32, 1>(Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_U32I32) {
    // array<u32, 1u>(1i, 0u, 0u, 0u, 0u, 0u);
    auto* tc =
        array<u32, 1>(Expr(Source{{12, 34}}, 1_i), Expr(0_u), Expr(0_u), Expr(0_u), Expr(0_u));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Array_ScalarArgumentTypeMismatch_Vec2) {
    // array<i32, 3u>(1i, vec2<i32>());
    auto* tc = array<i32, 3>(Expr(1_i), Construct(Source{{12, 34}}, ty.vec2<i32>()));
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'i32', found 'vec2<i32>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfVector_SubElemTypeMismatch_I32U32) {
    // array<vec3<i32>, 2u>(vec3<i32>(), vec3<u32>());
    auto* e0 = vec3<i32>();
    SetSource(Source::Location({12, 34}));
    auto* e1 = vec3<u32>();
    auto* t = Construct(ty.array(ty.vec3<i32>(), 2_i), e0, e1);
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'vec3<i32>', found 'vec3<u32>'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_ArrayOfVector_SubElemTypeMismatch_I32Bool) {
    // array<vec3<i32>, 2u>(vec3<i32>(), vec3<bool>(true, true, false));
    SetSource(Source::Location({12, 34}));
    auto* e0 = vec3<bool>(true, true, false);
    auto* e1 = vec3<i32>();
    auto* t = Construct(ty.array(ty.vec3<i32>(), 2_i), e0, e1);
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'vec3<i32>', found 'vec3<bool>'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_ArrayOfArray_SubElemSizeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 3u>(), array<i32, 2u>());
    SetSource(Source::Location({12, 34}));
    auto* e0 = array<i32, 3>();
    auto* e1 = array<i32, 2>();
    auto* t = Construct(ty.array(ty.array<i32, 2>(), 2_i), e0, e1);
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'array<i32, 2>', found 'array<i32, 3>'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_ArrayOfArray_SubElemTypeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 2u>(), array<u32, 2u>());
    auto* e0 = array<i32, 2>();
    SetSource(Source::Location({12, 34}));
    auto* e1 = array<u32, 2>();
    auto* t = Construct(ty.array(ty.array<i32, 2>(), 2_i), e0, e1);
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in array constructor does not match array type: "
              "expected 'array<i32, 2>', found 'array<u32, 2>'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_TooFewElements) {
    // array<i32, 4u>(1i, 2i, 3i);
    SetSource(Source::Location({12, 34}));
    auto* tc = array<i32, 4>(Expr(1_i), Expr(2_i), Expr(3_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array constructor has too few elements: expected 4, "
              "found 3");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_TooManyElements) {
    // array<i32, 4u>(1i, 2i, 3i, 4i, 5i);
    SetSource(Source::Location({12, 34}));
    auto* tc = array<i32, 4>(Expr(1_i), Expr(2_i), Expr(3_i), Expr(4_i), Expr(5_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array constructor has too many "
              "elements: expected 4, "
              "found 5");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_Runtime) {
    // array<i32>(1i);
    auto* tc = array(ty.i32(), nullptr, Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: cannot init a runtime-sized array");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Array_RuntimeZeroValue) {
    // array<i32>();
    auto* tc = array(ty.i32(), nullptr);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: cannot init a runtime-sized array");
}

}  // namespace ArrayConstructor

namespace ScalarConstructor {

TEST_F(ResolverTypeConstructorValidationTest, Expr_Construct_i32_Success) {
    auto* expr = Construct<i32>(Expr(123_i));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::I32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Construct_u32_Success) {
    auto* expr = Construct<u32>(Expr(123_u));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::U32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::U32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Construct_f32_Success) {
    auto* expr = Construct<f32>(Expr(1.23f));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::F32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::F32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Convert_f32_to_i32_Success) {
    auto* expr = Construct<i32>(1.23f);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::I32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::F32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Convert_i32_to_u32_Success) {
    auto* expr = Construct<u32>(123_i);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::U32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Convert_u32_to_f32_Success) {
    auto* expr = Construct<f32>(123_u);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::F32>());

    auto* call = Sem().Get(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::U32>());
}

}  // namespace ScalarConstructor

namespace VectorConstructor {

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2F32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec2<f32>(Expr(Source{{12, 34}}, 1_i), 1.0f);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2U32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec2<u32>(1_u, Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2I32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec2<i32>(Expr(Source{{12, 34}}, 1_u), 1_i);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2Bool_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec2<bool>(true, Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_Vec3ArgumentCardinalityTooLarge) {
    auto* tc = vec2<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_Vec4ArgumentCardinalityTooLarge) {
    auto* tc = vec2<f32>(Construct(Source{{12, 34}}, ty.vec4<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Error_TooManyArgumentsScalar) {
    auto* tc = vec2<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Error_TooManyArgumentsVector) {
    auto* tc = vec2<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsVectorAndScalar) {
    auto* tc = vec2<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()), Expr(Source{{12, 40}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Error_InvalidArgumentType) {
    auto* tc = vec2<f32>(Construct(Source{{12, 34}}, ty.mat2x2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected vector or scalar type in vector "
              "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Success_ZeroValue) {
    auto* tc = vec2<f32>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 0u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2F32_Success_Scalar) {
    auto* tc = vec2<f32>(1.0f, 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::F32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2U32_Success_Scalar) {
    auto* tc = vec2<u32>(1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::U32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2I32_Success_Scalar) {
    auto* tc = vec2<i32>(1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2Bool_Success_Scalar) {
    auto* tc = vec2<bool>(true, false);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Bool>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::Bool>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Success_Identity) {
    auto* tc = vec2<f32>(vec2<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec2_Success_Vec2TypeConversion) {
    auto* tc = vec2<f32>(vec2<i32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3F32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec3<f32>(1.0f, 1.0f, Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3U32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec3<u32>(1_u, Expr(Source{{12, 34}}, 1_i), 1_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3I32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec3<i32>(1_i, Expr(Source{{12, 34}}, 1_u), 1_i);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3Bool_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec3<bool>(true, Expr(Source{{12, 34}}, 1_i), false);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_Vec4ArgumentCardinalityTooLarge) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.vec4<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_TooFewArgumentsScalar) {
    auto* tc = vec3<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 40}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_TooManyArgumentsScalar) {
    auto* tc = vec3<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f), Expr(Source{{12, 52}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_TooFewArgumentsVec2) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_TooManyArgumentsVec2) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec2AndScalar) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_TooManyArgumentsVec3) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()), Expr(Source{{12, 40}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Error_InvalidArgumentType) {
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, ty.mat2x2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected vector or scalar type in vector "
              "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Success_ZeroValue) {
    auto* tc = vec3<f32>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 0u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3F32_Success_Scalar) {
    auto* tc = vec3<f32>(1.0f, 1.0f, 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::F32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::F32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3U32_Success_Scalar) {
    auto* tc = vec3<u32>(1_u, 1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::U32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3I32_Success_Scalar) {
    auto* tc = vec3<i32>(1_i, 1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3Bool_Success_Scalar) {
    auto* tc = vec3<bool>(true, false, true);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Bool>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::Bool>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::Bool>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Success_Vec2AndScalar) {
    auto* tc = vec3<f32>(vec2<f32>(), 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::F32>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Success_ScalarAndVec2) {
    auto* tc = vec3<f32>(1.0f, vec2<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::Vector>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Success_Identity) {
    auto* tc = vec3<f32>(vec3<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec3_Success_Vec3TypeConversion) {
    auto* tc = vec3<f32>(vec3<i32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);

    auto* call = Sem().Get(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::TypeConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().size(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4F32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec4<f32>(1.0f, 1.0f, Expr(Source{{12, 34}}, 1_i), 1.0f);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4U32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec4<u32>(1_u, 1_u, Expr(Source{{12, 34}}, 1_i), 1_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4I32_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec4<i32>(1_i, 1_i, Expr(Source{{12, 34}}, 1_u), 1_i);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4Bool_Error_ScalarArgumentTypeMismatch) {
    auto* tc = vec4<bool>(true, false, Expr(Source{{12, 34}}, 1_i), true);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Error_TooFewArgumentsScalar) {
    auto* tc = vec4<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Error_TooManyArgumentsScalar) {
    auto* tc = vec4<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f), Expr(Source{{12, 52}}, 1.0f),
                         Expr(Source{{12, 58}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsVec2AndScalar) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()), Expr(Source{{12, 40}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndScalars) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f), Expr(Source{{12, 52}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Scalar) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()), Expr(Source{{12, 46}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Vec2) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Error_TooFewArgumentsVec3) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndScalars) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()), Expr(Source{{12, 40}}, 1.0f),
                         Expr(Source{{12, 46}}, 1.0f));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec2) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()),
                         Construct(Source{{12, 40}}, ty.vec2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndVec3) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec2<f32>()),
                         Construct(Source{{12, 40}}, ty.vec3<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec3) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.vec3<f32>()),
                         Construct(Source{{12, 40}}, ty.vec3<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Error_InvalidArgumentType) {
    auto* tc = vec4<f32>(Construct(Source{{12, 34}}, ty.mat2x2<f32>()));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected vector or scalar type in vector "
              "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_ZeroValue) {
    auto* tc = vec4<f32>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4F32_Success_Scalar) {
    auto* tc = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4U32_Success_Scalar) {
    auto* tc = vec4<u32>(1_u, 1_u, 1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4I32_Success_Scalar) {
    auto* tc = vec4<i32>(1_i, 1_i, 1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4Bool_Success_Scalar) {
    auto* tc = vec4<bool>(true, false, true, false);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_Vec2ScalarScalar) {
    auto* tc = vec4<f32>(vec2<f32>(), 1.0f, 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_ScalarVec2Scalar) {
    auto* tc = vec4<f32>(1.0f, vec2<f32>(), 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_ScalarScalarVec2) {
    auto* tc = vec4<f32>(1.0f, 1.0f, vec2<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_Vec2AndVec2) {
    auto* tc = vec4<f32>(vec2<f32>(), vec2<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_Vec3AndScalar) {
    auto* tc = vec4<f32>(vec3<f32>(), 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_ScalarAndVec3) {
    auto* tc = vec4<f32>(1.0f, vec3<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_Identity) {
    auto* tc = vec4<f32>(vec4<f32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vec4_Success_Vec4TypeConversion) {
    auto* tc = vec4<f32>(vec4<i32>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_NestedVectorConstructors_InnerError) {
    auto* tc =
        vec4<f32>(vec4<f32>(1.0f, 1.0f,
                            vec3<f32>(Expr(Source{{12, 34}}, 1.0f), Expr(Source{{12, 34}}, 1.0f))),
                  1.0f);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_NestedVectorConstructors_Success) {
    auto* tc = vec4<f32>(vec3<f32>(vec2<f32>(1.0f, 1.0f), 1.0f), 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vector_Alias_Argument_Error) {
    auto* alias = Alias("UnsignedInt", ty.u32());
    Global("uint_var", ty.Of(alias), ast::StorageClass::kPrivate);

    auto* tc = vec2<f32>(Expr(Source{{12, 34}}, "uint_var"));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type in vector constructor does not match vector "
              "type: expected 'f32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vector_Alias_Argument_Success) {
    auto* f32_alias = Alias("Float32", ty.f32());
    auto* vec2_alias = Alias("VectorFloat2", ty.vec2<f32>());
    Global("my_f32", ty.Of(f32_alias), ast::StorageClass::kPrivate);
    Global("my_vec2", ty.Of(vec2_alias), ast::StorageClass::kPrivate);

    auto* tc = vec3<f32>("my_vec2", "my_f32");
    WrapInFunction(tc);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vector_ElementTypeAlias_Error) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec2<Float32>(1.0f, 1u)
    auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
    auto* tc = Construct(Source{{12, 34}}, vec_type, 1.0f, Expr(Source{{12, 40}}, 1_u));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:40 error: type in vector constructor does not match vector "
              "type: expected 'f32', found 'u32'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Vector_ElementTypeAlias_Success) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec2<Float32>(1.0f, 1.0f)
    auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
    auto* tc = Construct(Source{{12, 34}}, vec_type, 1.0f, 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest,
       Expr_Constructor_Vector_ArgumentElementTypeAlias_Error) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec3<u32>(vec<Float32>(), 1.0f)
    auto* vec_type = ty.vec(ty.Of(f32_alias), 2);
    auto* tc = vec3<u32>(Construct(Source{{12, 34}}, vec_type), 1.0f);
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
    auto* tc = vec3<f32>(Construct(Source{{12, 34}}, vec_type), 1.0f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec2ElementTypeFromScalars) {
    auto* vec2_bool = Construct(create<ast::Vector>(nullptr, 2), Expr(true), Expr(false));
    auto* vec2_i32 = Construct(create<ast::Vector>(nullptr, 2), Expr(1_i), Expr(2_i));
    auto* vec2_u32 = Construct(create<ast::Vector>(nullptr, 2), Expr(1_u), Expr(2_u));
    auto* vec2_f32 = Construct(create<ast::Vector>(nullptr, 2), Expr(1.0f), Expr(2.0f));
    WrapInFunction(vec2_bool, vec2_i32, vec2_u32, vec2_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec2_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec2_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec2_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec2_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec2_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec2_bool)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_i32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_u32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_bool), TypeOf(vec2_bool->target.type));
    EXPECT_EQ(TypeOf(vec2_i32), TypeOf(vec2_i32->target.type));
    EXPECT_EQ(TypeOf(vec2_u32), TypeOf(vec2_u32->target.type));
    EXPECT_EQ(TypeOf(vec2_f32), TypeOf(vec2_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec2ElementTypeFromVec2) {
    auto* vec2_bool = Construct(create<ast::Vector>(nullptr, 2), vec2<bool>(true, false));
    auto* vec2_i32 = Construct(create<ast::Vector>(nullptr, 2), vec2<i32>(1_i, 2_i));
    auto* vec2_u32 = Construct(create<ast::Vector>(nullptr, 2), vec2<u32>(1_u, 2_u));
    auto* vec2_f32 = Construct(create<ast::Vector>(nullptr, 2), vec2<f32>(1.0f, 2.0f));
    WrapInFunction(vec2_bool, vec2_i32, vec2_u32, vec2_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec2_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec2_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec2_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec2_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec2_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec2_bool)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_i32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_u32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f32)->As<sem::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_bool), TypeOf(vec2_bool->target.type));
    EXPECT_EQ(TypeOf(vec2_i32), TypeOf(vec2_i32->target.type));
    EXPECT_EQ(TypeOf(vec2_u32), TypeOf(vec2_u32->target.type));
    EXPECT_EQ(TypeOf(vec2_f32), TypeOf(vec2_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec3ElementTypeFromScalars) {
    auto* vec3_bool =
        Construct(create<ast::Vector>(nullptr, 3), Expr(true), Expr(false), Expr(true));
    auto* vec3_i32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1_i), Expr(2_i), Expr(3_i));
    auto* vec3_u32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1_u), Expr(2_u), Expr(3_u));
    auto* vec3_f32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1.0f), Expr(2.0f), Expr(3.0f));
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_bool), TypeOf(vec3_bool->target.type));
    EXPECT_EQ(TypeOf(vec3_i32), TypeOf(vec3_i32->target.type));
    EXPECT_EQ(TypeOf(vec3_u32), TypeOf(vec3_u32->target.type));
    EXPECT_EQ(TypeOf(vec3_f32), TypeOf(vec3_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec3ElementTypeFromVec3) {
    auto* vec3_bool = Construct(create<ast::Vector>(nullptr, 3), vec3<bool>(true, false, true));
    auto* vec3_i32 = Construct(create<ast::Vector>(nullptr, 3), vec3<i32>(1_i, 2_i, 3_i));
    auto* vec3_u32 = Construct(create<ast::Vector>(nullptr, 3), vec3<u32>(1_u, 2_u, 3_u));
    auto* vec3_f32 = Construct(create<ast::Vector>(nullptr, 3), vec3<f32>(1.0f, 2.0f, 3.0f));
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_bool), TypeOf(vec3_bool->target.type));
    EXPECT_EQ(TypeOf(vec3_i32), TypeOf(vec3_i32->target.type));
    EXPECT_EQ(TypeOf(vec3_u32), TypeOf(vec3_u32->target.type));
    EXPECT_EQ(TypeOf(vec3_f32), TypeOf(vec3_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec3ElementTypeFromScalarAndVec2) {
    auto* vec3_bool =
        Construct(create<ast::Vector>(nullptr, 3), Expr(true), vec2<bool>(false, true));
    auto* vec3_i32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1_i), vec2<i32>(2_i, 3_i));
    auto* vec3_u32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1_u), vec2<u32>(2_u, 3_u));
    auto* vec3_f32 = Construct(create<ast::Vector>(nullptr, 3), Expr(1.0f), vec2<f32>(2.0f, 3.0f));
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_bool), TypeOf(vec3_bool->target.type));
    EXPECT_EQ(TypeOf(vec3_i32), TypeOf(vec3_i32->target.type));
    EXPECT_EQ(TypeOf(vec3_u32), TypeOf(vec3_u32->target.type));
    EXPECT_EQ(TypeOf(vec3_f32), TypeOf(vec3_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec4ElementTypeFromScalars) {
    auto* vec4_bool = Construct(create<ast::Vector>(nullptr, 4), Expr(true), Expr(false),
                                Expr(true), Expr(false));
    auto* vec4_i32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1_i), Expr(2_i), Expr(3_i), Expr(4_i));
    auto* vec4_u32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1_u), Expr(2_u), Expr(3_u), Expr(4_u));
    auto* vec4_f32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1.0f), Expr(2.0f), Expr(3.0f), Expr(4.0f));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_bool), TypeOf(vec4_bool->target.type));
    EXPECT_EQ(TypeOf(vec4_i32), TypeOf(vec4_i32->target.type));
    EXPECT_EQ(TypeOf(vec4_u32), TypeOf(vec4_u32->target.type));
    EXPECT_EQ(TypeOf(vec4_f32), TypeOf(vec4_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec4ElementTypeFromVec4) {
    auto* vec4_bool =
        Construct(create<ast::Vector>(nullptr, 4), vec4<bool>(true, false, true, false));
    auto* vec4_i32 = Construct(create<ast::Vector>(nullptr, 4), vec4<i32>(1_i, 2_i, 3_i, 4_i));
    auto* vec4_u32 = Construct(create<ast::Vector>(nullptr, 4), vec4<u32>(1_u, 2_u, 3_u, 4_u));
    auto* vec4_f32 = Construct(create<ast::Vector>(nullptr, 4), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_bool), TypeOf(vec4_bool->target.type));
    EXPECT_EQ(TypeOf(vec4_i32), TypeOf(vec4_i32->target.type));
    EXPECT_EQ(TypeOf(vec4_u32), TypeOf(vec4_u32->target.type));
    EXPECT_EQ(TypeOf(vec4_f32), TypeOf(vec4_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec4ElementTypeFromScalarAndVec3) {
    auto* vec4_bool =
        Construct(create<ast::Vector>(nullptr, 4), Expr(true), vec3<bool>(false, true, false));
    auto* vec4_i32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1_i), vec3<i32>(2_i, 3_i, 4_i));
    auto* vec4_u32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1_u), vec3<u32>(2_u, 3_u, 4_u));
    auto* vec4_f32 =
        Construct(create<ast::Vector>(nullptr, 4), Expr(1.0f), vec3<f32>(2.0f, 3.0f, 4.0f));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_bool), TypeOf(vec4_bool->target.type));
    EXPECT_EQ(TypeOf(vec4_i32), TypeOf(vec4_i32->target.type));
    EXPECT_EQ(TypeOf(vec4_u32), TypeOf(vec4_u32->target.type));
    EXPECT_EQ(TypeOf(vec4_f32), TypeOf(vec4_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, InferVec4ElementTypeFromVec2AndVec2) {
    auto* vec4_bool = Construct(create<ast::Vector>(nullptr, 4), vec2<bool>(true, false),
                                vec2<bool>(true, false));
    auto* vec4_i32 =
        Construct(create<ast::Vector>(nullptr, 4), vec2<i32>(1_i, 2_i), vec2<i32>(3_i, 4_i));
    auto* vec4_u32 =
        Construct(create<ast::Vector>(nullptr, 4), vec2<u32>(1_u, 2_u), vec2<u32>(3_u, 4_u));
    auto* vec4_f32 =
        Construct(create<ast::Vector>(nullptr, 4), vec2<f32>(1.0f, 2.0f), vec2<f32>(3.0f, 4.0f));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<sem::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_bool), TypeOf(vec4_bool->target.type));
    EXPECT_EQ(TypeOf(vec4_i32), TypeOf(vec4_i32->target.type));
    EXPECT_EQ(TypeOf(vec4_u32), TypeOf(vec4_u32->target.type));
    EXPECT_EQ(TypeOf(vec4_f32), TypeOf(vec4_f32->target.type));
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVectorElementTypeWithoutArgs) {
    WrapInFunction(Construct(create<ast::Vector>(Source{{12, 34}}, nullptr, 3)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing vector element type");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec2ElementTypeFromScalarsMismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 2),
                             Expr(Source{{1, 2}}, 1_i),  //
                             Expr(Source{{1, 3}}, 2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type i32
1:3 note: argument 1 has type u32)");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec3ElementTypeFromScalarsMismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 3),
                             Expr(Source{{1, 2}}, 1_i),  //
                             Expr(Source{{1, 3}}, 2_u),  //
                             Expr(Source{{1, 4}}, 3_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type i32
1:3 note: argument 1 has type u32
1:4 note: argument 2 has type i32)");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec3ElementTypeFromScalarAndVec2Mismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 3),
                             Expr(Source{{1, 2}}, 1_i),  //
                             Construct(Source{{1, 3}}, ty.vec2<f32>(), 2.0f, 3.0f)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type i32
1:3 note: argument 1 has type vec2<f32>)");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec4ElementTypeFromScalarsMismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 4),
                             Expr(Source{{1, 2}}, 1_i),   //
                             Expr(Source{{1, 3}}, 2_i),   //
                             Expr(Source{{1, 4}}, 3.0f),  //
                             Expr(Source{{1, 5}}, 4_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type i32
1:3 note: argument 1 has type i32
1:4 note: argument 2 has type f32
1:5 note: argument 3 has type i32)");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec4ElementTypeFromScalarAndVec3Mismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 4),
                             Expr(Source{{1, 2}}, 1_i),  //
                             Construct(Source{{1, 3}}, ty.vec3<u32>(), 2_u, 3_u, 4_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type i32
1:3 note: argument 1 has type vec3<u32>)");
}

TEST_F(ResolverTypeConstructorValidationTest, CannotInferVec4ElementTypeFromVec2AndVec2Mismatch) {
    WrapInFunction(Construct(Source{{1, 1}}, create<ast::Vector>(nullptr, 4),
                             Construct(Source{{1, 2}}, ty.vec2<i32>(), 3_i, 4_i),  //
                             Construct(Source{{1, 3}}, ty.vec2<u32>(), 3_u, 4_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:1 error: cannot infer vector element type, as constructor arguments have different types
1:2 note: argument 0 has type vec2<i32>
1:3 note: argument 1 has type vec2<u32>)");
}

}  // namespace VectorConstructor

namespace MatrixConstructor {
struct MatrixDimensions {
    uint32_t rows;
    uint32_t columns;
};

static std::string MatrixStr(const MatrixDimensions& dimensions) {
    return "mat" + std::to_string(dimensions.columns) + "x" + std::to_string(dimensions.rows) +
           "<f32>";
}

using MatrixConstructorTest = ResolverTestWithParam<MatrixDimensions>;

TEST_P(MatrixConstructorTest, Expr_ColumnConstructor_Error_TooFewArguments) {
    // matNxM<f32>(vecM<f32>(), ...); with N - 1 arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns - 1; i++) {
        auto* vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<f32>";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ElementConstructor_Error_TooFewArguments) {
    // matNxM<f32>(f32,...,f32); with N*M - 1 arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns * param.rows - 1; i++) {
        args.push_back(Construct(Source{{12, i}}, ty.f32()));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "f32";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ColumnConstructor_Error_TooManyArguments) {
    // matNxM<f32>(vecM<f32>(), ...); with N + 1 arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns + 1; i++) {
        auto* vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<f32>";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ElementConstructor_Error_TooManyArguments) {
    // matNxM<f32>(f32,...,f32); with N*M + 1 arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns * param.rows + 1; i++) {
        args.push_back(Construct(Source{{12, i}}, ty.f32()));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "f32";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ColumnConstructor_Error_InvalidArgumentType) {
    // matNxM<f32>(vec<u32>, vec<u32>, ...); N arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec<u32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ElementConstructor_Error_InvalidArgumentType) {
    // matNxM<f32>(u32, u32, ...); N*M arguments

    const auto param = GetParam();

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        args.push_back(Expr(Source{{12, i}}, 1_u));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "u32";
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ColumnConstructor_Error_TooFewRowsInVectorArgument) {
    // matNxM<f32>(vecM<f32>(),...,vecM-1<f32>());

    const auto param = GetParam();

    // Skip the test if parameters would have resulted in an invalid vec1 type.
    if (param.rows == 2) {
        return;
    }

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns - 1; i++) {
        auto* valid_vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, valid_vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<f32>";
    }
    const size_t kInvalidLoc = 2 * (param.columns - 1);
    auto* invalid_vec_type = ty.vec<f32>(param.rows - 1);
    args.push_back(Construct(Source{{12, kInvalidLoc}}, invalid_vec_type));
    args_tys << ", vec" << (param.rows - 1) << "<f32>";

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_ColumnConstructor_Error_TooManyRowsInVectorArgument) {
    // matNxM<f32>(vecM<f32>(),...,vecM+1<f32>());

    const auto param = GetParam();

    // Skip the test if parameters would have resulted in an invalid vec5 type.
    if (param.rows == 4) {
        return;
    }

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns - 1; i++) {
        auto* valid_vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, valid_vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<f32>";
    }
    const size_t kInvalidLoc = 2 * (param.columns - 1);
    auto* invalid_vec_type = ty.vec<f32>(param.rows + 1);
    args.push_back(Construct(Source{{12, kInvalidLoc}}, invalid_vec_type));
    args_tys << ", vec" << (param.rows + 1) << "<f32>";

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ZeroValue_Success) {
    // matNxM<f32>();

    const auto param = GetParam();
    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{{12, 40}}, matrix_type);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_WithColumns_Success) {
    // matNxM<f32>(vecM<f32>(), ...); with N arguments

    const auto param = GetParam();

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_WithElements_Success) {
    // matNxM<f32>(f32,...,f32); with N*M arguments

    const auto param = GetParam();

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns * param.rows; i++) {
        args.push_back(Construct(Source{{12, i}}, ty.f32()));
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Error) {
    // matNxM<Float32>(vecM<u32>(), ...); with N arguments

    const auto param = GetParam();
    auto* f32_alias = Alias("Float32", ty.f32());

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec(ty.u32(), param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    auto* matrix_type = ty.mat(ty.Of(f32_alias), param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Success) {
    // matNxM<Float32>(vecM<f32>(), ...); with N arguments

    const auto param = GetParam();
    auto* f32_alias = Alias("Float32", ty.f32());

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec<f32>(param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
    }

    auto* matrix_type = ty.mat(ty.Of(f32_alias), param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_MatrixConstructor_ArgumentTypeAlias_Error) {
    auto* alias = Alias("VectorUnsigned2", ty.vec2<u32>());
    auto* tc = mat2x2<f32>(Construct(Source{{12, 34}}, ty.Of(alias)), vec2<f32>());
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching constructor mat2x2<f32>(vec2<u32>, vec2<f32>)

3 candidates available:
  mat2x2<f32>()
  mat2x2<f32>(f32,...,f32) // 4 arguments
  mat2x2<f32>(vec2<f32>, vec2<f32>)
)");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentTypeAlias_Success) {
    const auto param = GetParam();
    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* vec_type = ty.vec<f32>(param.rows);
    auto* vec_alias = Alias("VectorFloat2", vec_type);

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        args.push_back(Construct(Source{{12, i}}, ty.Of(vec_alias)));
    }

    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentElementTypeAlias_Error) {
    const auto param = GetParam();
    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* f32_alias = Alias("UnsignedInt", ty.u32());

    std::stringstream args_tys;
    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec(ty.Of(f32_alias), param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
        if (i > 1) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:1 error: no matching constructor " + MatrixStr(param) +
                                        "(" + args_tys.str() + ")\n\n3 candidates available:"));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentElementTypeAlias_Success) {
    const auto param = GetParam();
    auto* f32_alias = Alias("Float32", ty.f32());

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        auto* vec_type = ty.vec(ty.Of(f32_alias), param.rows);
        args.push_back(Construct(Source{{12, i}}, vec_type));
    }

    auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, InferElementTypeFromVectors) {
    const auto param = GetParam();

    ast::ExpressionList args;
    for (uint32_t i = 1; i <= param.columns; i++) {
        args.push_back(Construct(ty.vec<f32>(param.rows)));
    }

    auto* matrix_type = create<ast::Matrix>(nullptr, param.rows, param.columns);
    auto* tc = Construct(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, InferElementTypeFromScalars) {
    const auto param = GetParam();

    ast::ExpressionList args;
    for (uint32_t i = 0; i < param.rows * param.columns; i++) {
        args.push_back(Expr(static_cast<f32>(i)));
    }

    auto* matrix_type = create<ast::Matrix>(nullptr, param.rows, param.columns);
    WrapInFunction(Construct(Source{{12, 34}}, matrix_type, std::move(args)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, CannotInferElementTypeFromVectors_Mismatch) {
    const auto param = GetParam();

    std::stringstream err;
    err << "12:34 error: cannot infer matrix element type, as constructor "
           "arguments have different types";

    ast::ExpressionList args;
    for (uint32_t i = 0; i < param.columns; i++) {
        err << "\n";
        auto src = Source{{1, 10 + i}};
        if (i == 1) {
            // Odd one out
            args.push_back(Construct(src, ty.vec<i32>(param.rows)));
            err << src << " note: argument " << i << " has type vec" << param.rows << "<i32>";
        } else {
            args.push_back(Construct(src, ty.vec<f32>(param.rows)));
            err << src << " note: argument " << i << " has type vec" << param.rows << "<f32>";
        }
    }

    auto* matrix_type = create<ast::Matrix>(nullptr, param.rows, param.columns);
    WrapInFunction(Construct(Source{{12, 34}}, matrix_type, std::move(args)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), err.str());
}

TEST_P(MatrixConstructorTest, CannotInferElementTypeFromScalars_Mismatch) {
    const auto param = GetParam();

    std::stringstream err;
    err << "12:34 error: cannot infer matrix element type, as constructor "
           "arguments have different types";
    ast::ExpressionList args;
    for (uint32_t i = 0; i < param.rows * param.columns; i++) {
        err << "\n";
        auto src = Source{{1, 10 + i}};
        if (i == 3) {
            args.push_back(Expr(src, static_cast<i32>(i)));  // The odd one out
            err << src << " note: argument " << i << " has type i32";
        } else {
            args.push_back(Expr(src, static_cast<f32>(i)));
            err << src << " note: argument " << i << " has type f32";
        }
    }

    auto* matrix_type = create<ast::Matrix>(nullptr, param.rows, param.columns);
    WrapInFunction(Construct(Source{{12, 34}}, matrix_type, std::move(args)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), err.str());
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
using builder::mat2x2;
using builder::mat3x3;
using builder::mat4x4;
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
    auto* tc = Construct(Source{{12, 34}}, ty.Of(s), values);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: struct constructor has too few inputs: expected " +
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
    auto* tc = Construct(Source{{12, 34}}, ty.Of(s), values);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: struct constructor has too many inputs: expected " +
                                std::to_string(N) + ", found " + std::to_string(N + 1));
}

INSTANTIATE_TEST_SUITE_P(ResolverTypeConstructorValidationTest,
                         StructConstructorInputsTest,
                         testing::Combine(testing::ValuesIn(all_types), number_of_members));
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
    auto* tc = Construct(ty.Of(s), values);
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

    auto* tc = Construct(Source{{12, 34}}, ty.Of(s), 1_i, 1_i, 1_i);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: type in struct constructor does not match struct member "
              "type: expected 'inner_s', found 'i32'");
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Struct) {
    auto* m = Member("m", ty.i32());
    auto* s = Structure("MyInputs", {m});
    auto* tc = Construct(Source{{12, 34}}, ty.Of(s));
    WrapInFunction(tc);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeConstructorValidationTest, Expr_Constructor_Struct_Empty) {
    auto* str = Structure("S", {
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Construct(ty.Of(str)));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace StructConstructor

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_Atomic) {
    WrapInFunction(Assign(Phony(), Construct(Source{{12, 34}}, ty.atomic(ty.i32()))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_AtomicArray) {
    WrapInFunction(
        Assign(Phony(), Construct(Source{{12, 34}}, ty.array(ty.atomic(ty.i32()), 4_i))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array constructor has non-constructible element type");
}

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_AtomicStructMember) {
    auto* str = Structure("S", {Member("a", ty.atomic(ty.i32()))});
    WrapInFunction(Assign(Phony(), Construct(Source{{12, 34}}, ty.Of(str))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: struct constructor has non-constructible type");
}

TEST_F(ResolverTypeConstructorValidationTest, NonConstructibleType_Sampler) {
    WrapInFunction(
        Assign(Phony(), Construct(Source{{12, 34}}, ty.sampler(ast::SamplerKind::kSampler))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

TEST_F(ResolverTypeConstructorValidationTest, TypeConstructorAsStatement) {
    WrapInFunction(CallStmt(Construct(Source{{12, 34}}, ty.vec2<f32>(), 1.f, 2.f)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type constructor evaluated but not used");
}

TEST_F(ResolverTypeConstructorValidationTest, TypeConversionAsStatement) {
    WrapInFunction(CallStmt(Construct(Source{{12, 34}}, ty.f32(), 1_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type cast evaluated but not used");
}

}  // namespace
}  // namespace tint::resolver
