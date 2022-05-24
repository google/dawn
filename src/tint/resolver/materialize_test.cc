// Copyright 2022 The Tint Authors.
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

#include "src/tint/sem/materialize.h"

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using AFloatV = builder::vec<3, AFloat>;
using AFloatM = builder::mat<3, 2, AFloat>;
using AIntV = builder::vec<3, AInt>;
using f32V = builder::vec<3, f32>;
using f16V = builder::vec<3, f16>;
using i32V = builder::vec<3, i32>;
using u32V = builder::vec<3, u32>;
using f32M = builder::mat<3, 2, f32>;

////////////////////////////////////////////////////////////////////////////////
// MaterializeTests
////////////////////////////////////////////////////////////////////////////////
namespace MaterializeTests {

// How should the materialization occur?
enum class Method {
    // var a : T = literal;
    kVar,

    // let a : T = literal;
    kLet,

    // fn F(v : T) {}
    // fn x() {
    //   F(literal);
    // }
    kFnArg,

    // min(target_expr, literal);
    kBuiltinArg,

    // fn F() : T {
    //   return literal;
    // }
    kReturn,

    // array<T, 1>(literal);
    kArray,

    // struct S {
    //   v : T
    // };
    // fn x() {
    //   _ = S(literal)
    // }
    kStruct,

    // target_expr + literal
    kBinaryOp,

    // switch (literal) {
    //   case target_expr: {}
    //   default: {}
    // }
    kSwitchCond,

    // switch (target_expr) {
    //   case literal: {}
    //   default: {}
    // }
    kSwitchCase,

    // switch (literal) {
    //   case 123: {}
    //   case target_expr: {}
    //   default: {}
    // }
    kSwitchCondWithAbstractCase,

    // switch (target_expr) {
    //   case 123: {}
    //   case literal: {}
    //   default: {}
    // }
    kSwitchCaseWithAbstractCase,
};

static std::ostream& operator<<(std::ostream& o, Method m) {
    switch (m) {
        case Method::kVar:
            return o << "var";
        case Method::kLet:
            return o << "let";
        case Method::kFnArg:
            return o << "fn-arg";
        case Method::kBuiltinArg:
            return o << "builtin-arg";
        case Method::kReturn:
            return o << "return";
        case Method::kArray:
            return o << "array";
        case Method::kStruct:
            return o << "struct";
        case Method::kBinaryOp:
            return o << "binary-op";
        case Method::kSwitchCond:
            return o << "switch-cond";
        case Method::kSwitchCase:
            return o << "switch-case";
        case Method::kSwitchCondWithAbstractCase:
            return o << "switch-cond-with-abstract";
        case Method::kSwitchCaseWithAbstractCase:
            return o << "switch-case-with-abstract";
    }
    return o << "<unknown>";
}

struct Data {
    std::string target_type_name;
    builder::ast_type_func_ptr target_ast_ty;
    builder::sem_type_func_ptr target_sem_ty;
    builder::ast_expr_func_ptr target_expr;
    std::string literal_type_name;
    builder::ast_expr_func_ptr literal_value;
    std::variant<AInt, AFloat> materialized_value;
};

template <typename TARGET_TYPE, typename LITERAL_TYPE, typename MATERIALIZED_TYPE = AInt>
Data Types(MATERIALIZED_TYPE materialized_value = 0_a) {
    return {
        builder::DataType<TARGET_TYPE>::Name(),   //
        builder::DataType<TARGET_TYPE>::AST,      //
        builder::DataType<TARGET_TYPE>::Sem,      //
        builder::DataType<TARGET_TYPE>::Expr,     //
        builder::DataType<LITERAL_TYPE>::Name(),  //
        builder::DataType<LITERAL_TYPE>::Expr,    //
        materialized_value,
    };
}

static std::ostream& operator<<(std::ostream& o, const Data& c) {
    return o << "[" << c.target_type_name << " <- " << c.literal_type_name << "]";
}

enum class Expectation {
    kMaterialize,
    kNoMaterialize,
    kInvalidCast,
};

static std::ostream& operator<<(std::ostream& o, Expectation m) {
    switch (m) {
        case Expectation::kMaterialize:
            return o << "pass";
        case Expectation::kNoMaterialize:
            return o << "no-materialize";
        case Expectation::kInvalidCast:
            return o << "invalid-cast";
    }
    return o << "<unknown>";
}

using MaterializeAbstractNumeric =
    resolver::ResolverTestWithParam<std::tuple<Expectation, Method, Data>>;

TEST_P(MaterializeAbstractNumeric, Test) {
    // Once F16 is properly supported, we'll need to enable this:
    // Enable(ast::Extension::kF16);

    const auto& param = GetParam();
    const auto& expectation = std::get<0>(param);
    const auto& method = std::get<1>(param);
    const auto& data = std::get<2>(param);

    auto target_ty = [&] { return data.target_ast_ty(*this); };
    auto target_expr = [&] { return data.target_expr(*this, 42); };
    auto* literal = data.literal_value(*this, 1);
    switch (method) {
        case Method::kVar:
            WrapInFunction(Decl(Var("a", target_ty(), literal)));
            break;
        case Method::kLet:
            WrapInFunction(Decl(Let("a", target_ty(), literal)));
            break;
        case Method::kFnArg:
            Func("F", {Param("P", target_ty())}, ty.void_(), {});
            WrapInFunction(CallStmt(Call("F", literal)));
            break;
        case Method::kBuiltinArg:
            WrapInFunction(CallStmt(Call("min", target_expr(), literal)));
            break;
        case Method::kReturn:
            Func("F", {}, target_ty(), {Return(literal)});
            break;
        case Method::kArray:
            WrapInFunction(Construct(ty.array(target_ty(), 1_i), literal));
            break;
        case Method::kStruct:
            Structure("S", {Member("v", target_ty())});
            WrapInFunction(Construct(ty.type_name("S"), literal));
            break;
        case Method::kBinaryOp:
            WrapInFunction(Add(target_expr(), literal));
            break;
        case Method::kSwitchCond:
            WrapInFunction(Switch(literal,                                               //
                                  Case(target_expr()->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCase:
            WrapInFunction(Switch(target_expr(),                                   //
                                  Case(literal->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCondWithAbstractCase:
            WrapInFunction(Switch(literal,                                               //
                                  Case(Expr(123_a)),                                     //
                                  Case(target_expr()->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCaseWithAbstractCase:
            WrapInFunction(Switch(target_expr(),                                   //
                                  Case(Expr(123_a)),                               //
                                  Case(literal->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
    }

    auto check_types_and_values = [&](const sem::Expression* expr) {
        auto* target_sem_ty = data.target_sem_ty(*this);

        EXPECT_TYPE(expr->Type(), target_sem_ty);
        EXPECT_TYPE(expr->ConstantValue().Type(), target_sem_ty);

        uint32_t num_elems = 0;
        const sem::Type* target_sem_el_ty = sem::Type::ElementOf(target_sem_ty, &num_elems);
        EXPECT_TYPE(expr->ConstantValue().ElementType(), target_sem_el_ty);
        std::visit(
            [&](auto&& v) {
                EXPECT_EQ(expr->ConstantValue().Elements(), sem::Constant::Scalars(num_elems, {v}));
            },
            data.materialized_value);
    };

    switch (expectation) {
        case Expectation::kMaterialize: {
            ASSERT_TRUE(r()->Resolve()) << r()->error();
            auto* materialize = Sem().Get<sem::Materialize>(literal);
            ASSERT_NE(materialize, nullptr);
            check_types_and_values(materialize);
            break;
        }
        case Expectation::kNoMaterialize: {
            ASSERT_TRUE(r()->Resolve()) << r()->error();
            auto* sem = Sem().Get(literal);
            ASSERT_NE(sem, nullptr);
            EXPECT_FALSE(sem->Is<sem::Materialize>());
            check_types_and_values(sem);
            break;
        }
        case Expectation::kInvalidCast: {
            ASSERT_FALSE(r()->Resolve());
            std::string expect;
            switch (method) {
                case Method::kBuiltinArg:
                    expect = "error: no matching call to min(" + data.target_type_name + ", " +
                             data.literal_type_name + ")";
                    break;
                case Method::kBinaryOp:
                    expect = "error: no matching overload for operator + (" +
                             data.target_type_name + ", " + data.literal_type_name + ")";
                    break;
                default:
                    expect = "error: cannot convert value of type '" + data.literal_type_name +
                             "' to type '" + data.target_type_name + "'";
                    break;
            }
            EXPECT_THAT(r()->error(), testing::StartsWith(expect));
            break;
        }
    }
}

// TODO(crbug.com/tint/1504): Test for abstract-numeric values not fitting in materialized types.

INSTANTIATE_TEST_SUITE_P(MaterializeScalar,
                         MaterializeAbstractNumeric,                                        //
                         testing::Combine(testing::Values(Expectation::kMaterialize),       //
                                          testing::Values(Method::kLet,                     //
                                                          Method::kVar,                     //
                                                          Method::kFnArg,                   //
                                                          Method::kBuiltinArg,              //
                                                          Method::kReturn,                  //
                                                          Method::kArray,                   //
                                                          Method::kStruct,                  //
                                                          Method::kBinaryOp),               //
                                          testing::Values(Types<i32, AInt>(1_a),            //
                                                          Types<u32, AInt>(1_a),            //
                                                          Types<f32, AFloat>(1.0_a)         //
                                                          /* Types<f16, AFloat>(1.0_a), */  //
                                                          /* Types<f16, AFloat>(1.0_a), */)));

INSTANTIATE_TEST_SUITE_P(MaterializeVector,
                         MaterializeAbstractNumeric,                                          //
                         testing::Combine(testing::Values(Expectation::kMaterialize),         //
                                          testing::Values(Method::kLet,                       //
                                                          Method::kVar,                       //
                                                          Method::kFnArg,                     //
                                                          Method::kBuiltinArg,                //
                                                          Method::kReturn,                    //
                                                          Method::kArray,                     //
                                                          Method::kStruct,                    //
                                                          Method::kBinaryOp),                 //
                                          testing::Values(Types<i32V, AIntV>(1_a),            //
                                                          Types<u32V, AIntV>(1_a),            //
                                                          Types<f32V, AFloatV>(1.0_a)         //
                                                          /* Types<f16V, AFloatV>(1.0_a), */  //
                                                          /* Types<f16V, AFloatV>(1.0_a), */)));

INSTANTIATE_TEST_SUITE_P(MaterializeMatrix,
                         MaterializeAbstractNumeric,                                          //
                         testing::Combine(testing::Values(Expectation::kMaterialize),         //
                                          testing::Values(Method::kLet,                       //
                                                          Method::kVar,                       //
                                                          Method::kFnArg,                     //
                                                          Method::kReturn,                    //
                                                          Method::kArray,                     //
                                                          Method::kStruct,                    //
                                                          Method::kBinaryOp),                 //
                                          testing::Values(Types<f32M, AFloatM>(1.0_a)         //
                                                          /* Types<f16V, AFloatM>(1.0_a), */  //
                                                          )));

INSTANTIATE_TEST_SUITE_P(MaterializeSwitch,
                         MaterializeAbstractNumeric,                                             //
                         testing::Combine(testing::Values(Expectation::kMaterialize),            //
                                          testing::Values(Method::kSwitchCond,                   //
                                                          Method::kSwitchCase,                   //
                                                          Method::kSwitchCondWithAbstractCase,   //
                                                          Method::kSwitchCaseWithAbstractCase),  //
                                          testing::Values(Types<i32, AInt>(1_a),                 //
                                                          Types<u32, AInt>(1_a))));

// TODO(crbug.com/tint/1504): Enable once we have abstract overloads of builtins / binary ops.
INSTANTIATE_TEST_SUITE_P(DISABLED_NoMaterialize,
                         MaterializeAbstractNumeric,                                       //
                         testing::Combine(testing::Values(Expectation::kNoMaterialize),    //
                                          testing::Values(Method::kBuiltinArg,             //
                                                          Method::kBinaryOp),              //
                                          testing::Values(Types<AInt, AInt>(1_a),          //
                                                          Types<AFloat, AFloat>(1.0_a),    //
                                                          Types<AIntV, AIntV>(1_a),        //
                                                          Types<AFloatV, AFloatV>(1.0_a),  //
                                                          Types<AFloatM, AFloatM>(1.0_a))));
INSTANTIATE_TEST_SUITE_P(InvalidCast,
                         MaterializeAbstractNumeric,                                   //
                         testing::Combine(testing::Values(Expectation::kInvalidCast),  //
                                          testing::Values(Method::kLet,                //
                                                          Method::kVar,                //
                                                          Method::kFnArg,              //
                                                          Method::kBuiltinArg,         //
                                                          Method::kReturn,             //
                                                          Method::kArray,              //
                                                          Method::kStruct,             //
                                                          Method::kBinaryOp),          //
                                          testing::Values(Types<i32, AFloat>(),        //
                                                          Types<u32, AFloat>(),        //
                                                          Types<i32V, AFloatV>(),      //
                                                          Types<u32V, AFloatV>())));

}  // namespace MaterializeTests

}  // namespace
}  // namespace tint::resolver
