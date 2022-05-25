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
    std::string target_element_type_name;
    builder::ast_type_func_ptr target_ast_ty;
    builder::sem_type_func_ptr target_sem_ty;
    builder::ast_expr_func_ptr target_expr;
    std::string source_type_name;
    builder::ast_expr_func_ptr source_builder;
    std::variant<AInt, AFloat> materialized_value;
    double literal_value;
};

template <typename TARGET_TYPE, typename SOURCE_TYPE, typename MATERIALIZED_TYPE>
Data Types(MATERIALIZED_TYPE materialized_value, double literal_value) {
    using TargetDataType = builder::DataType<TARGET_TYPE>;
    using SourceDataType = builder::DataType<SOURCE_TYPE>;
    using TargetElementDataType = builder::DataType<typename TargetDataType::ElementType>;
    return {
        TargetDataType::Name(),         // target_type_name
        TargetElementDataType::Name(),  // target_element_type_name
        TargetDataType::AST,            // target_ast_ty
        TargetDataType::Sem,            // target_sem_ty
        TargetDataType::Expr,           // target_expr
        SourceDataType::Name(),         // literal_type_name
        SourceDataType::Expr,           // literal_builder
        materialized_value,
        literal_value,
    };
}

template <typename TARGET_TYPE, typename SOURCE_TYPE>
Data Types() {
    using TargetDataType = builder::DataType<TARGET_TYPE>;
    using SourceDataType = builder::DataType<SOURCE_TYPE>;
    using TargetElementDataType = builder::DataType<typename TargetDataType::ElementType>;
    return {
        TargetDataType::Name(),         // target_type_name
        TargetElementDataType::Name(),  // target_element_type_name
        TargetDataType::AST,            // target_ast_ty
        TargetDataType::Sem,            // target_sem_ty
        TargetDataType::Expr,           // target_expr
        SourceDataType::Name(),         // literal_type_name
        SourceDataType::Expr,           // literal_builder
        0_a,
        0.0,
    };
}

static std::ostream& operator<<(std::ostream& o, const Data& c) {
    auto print_value = [&](auto&& v) { o << v; };
    o << "[" << c.target_type_name << " <- " << c.source_type_name << "] [";
    std::visit(print_value, c.materialized_value);
    o << " <- " << c.literal_value << "]";
    return o;
}

enum class Expectation {
    kMaterialize,
    kNoMaterialize,
    kInvalidCast,
    kValueCannotBeRepresented,
};

static std::ostream& operator<<(std::ostream& o, Expectation m) {
    switch (m) {
        case Expectation::kMaterialize:
            return o << "pass";
        case Expectation::kNoMaterialize:
            return o << "no-materialize";
        case Expectation::kInvalidCast:
            return o << "invalid-cast";
        case Expectation::kValueCannotBeRepresented:
            return o << "value too low or high";
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
    auto* literal = data.source_builder(*this, data.literal_value);
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
        expr->ConstantValue().WithElements([&](auto&& vec) {
            using VEC_TY = std::decay_t<decltype(vec)>;
            using EL_TY = typename VEC_TY::value_type;
            ASSERT_TRUE(std::holds_alternative<EL_TY>(data.materialized_value));
            VEC_TY expected(num_elems, std::get<EL_TY>(data.materialized_value));
            EXPECT_EQ(vec, expected);
        });
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
                             data.source_type_name + ")";
                    break;
                case Method::kBinaryOp:
                    expect = "error: no matching overload for operator + (" +
                             data.target_type_name + ", " + data.source_type_name + ")";
                    break;
                default:
                    expect = "error: cannot convert value of type '" + data.source_type_name +
                             "' to type '" + data.target_type_name + "'";
                    break;
            }
            EXPECT_THAT(r()->error(), testing::StartsWith(expect));
            break;
        }
        case Expectation::kValueCannotBeRepresented:
            ASSERT_FALSE(r()->Resolve());
            EXPECT_THAT(r()->error(), testing::HasSubstr("cannot be represented as '" +
                                                         data.target_element_type_name + "'"));
            break;
    }
}

/// Methods that support scalar materialization
constexpr Method kScalarMethods[] = {Method::kLet,         //
                                     Method::kVar,         //
                                     Method::kFnArg,       //
                                     Method::kBuiltinArg,  //
                                     Method::kReturn,      //
                                     Method::kArray,       //
                                     Method::kStruct,      //
                                     Method::kBinaryOp};

/// Methods that support vector materialization
constexpr Method kVectorMethods[] = {Method::kLet,         //
                                     Method::kVar,         //
                                     Method::kFnArg,       //
                                     Method::kBuiltinArg,  //
                                     Method::kReturn,      //
                                     Method::kArray,       //
                                     Method::kStruct,      //
                                     Method::kBinaryOp};

/// Methods that support matrix materialization
constexpr Method kMatrixMethods[] = {Method::kLet,     //
                                     Method::kVar,     //
                                     Method::kFnArg,   //
                                     Method::kReturn,  //
                                     Method::kArray,   //
                                     Method::kStruct,  //
                                     Method::kBinaryOp};

/// Methods that support materialization for switch cases
constexpr Method kSwitchMethods[] = {Method::kSwitchCond,                  //
                                     Method::kSwitchCase,                  //
                                     Method::kSwitchCondWithAbstractCase,  //
                                     Method::kSwitchCaseWithAbstractCase};

constexpr double kMaxF32 = static_cast<double>(f32::kHighest);
constexpr double kPiF64 = 3.141592653589793;
constexpr double kPiF32 = 3.1415927410125732;  // kPiF64 quantized to f32

// (2^-127)×(1+(0xfffffffffffff÷0x10000000000000))
constexpr double kTooSmallF32 = 1.1754943508222874e-38;

INSTANTIATE_TEST_SUITE_P(
    MaterializeScalar,
    MaterializeAbstractNumeric,                                                       //
    testing::Combine(testing::Values(Expectation::kMaterialize),                      //
                     testing::ValuesIn(kScalarMethods),                               //
                     testing::Values(Types<i32, AInt>(0_a, 0.0),                      //
                                     Types<i32, AInt>(2147483647_a, 2147483647.0),    //
                                     Types<i32, AInt>(-2147483648_a, -2147483648.0),  //
                                     Types<u32, AInt>(0_a, 0.0),                      //
                                     Types<u32, AInt>(4294967295_a, 4294967295.0),    //
                                     Types<f32, AFloat>(0.0_a, 0.0),                  //
                                     Types<f32, AFloat>(AFloat(kMaxF32), kMaxF32),    //
                                     Types<f32, AFloat>(AFloat(-kMaxF32), -kMaxF32),  //
                                     Types<f32, AFloat>(AFloat(kPiF32), kPiF64),      //
                                     Types<f32, AFloat>(0.0_a, kTooSmallF32),         //
                                     Types<f32, AFloat>(-0.0_a, -kTooSmallF32)        //
                                     /* Types<f16, AFloat>(1.0_a), */                 //
                                     /* Types<f16, AFloat>(1.0_a), */)));

INSTANTIATE_TEST_SUITE_P(
    MaterializeVector,
    MaterializeAbstractNumeric,                                                         //
    testing::Combine(testing::Values(Expectation::kMaterialize),                        //
                     testing::ValuesIn(kVectorMethods),                                 //
                     testing::Values(Types<i32V, AIntV>(0_a, 0.0),                      //
                                     Types<i32V, AIntV>(2147483647_a, 2147483647.0),    //
                                     Types<i32V, AIntV>(-2147483648_a, -2147483648.0),  //
                                     Types<u32V, AIntV>(0_a, 0.0),                      //
                                     Types<u32V, AIntV>(4294967295_a, 4294967295.0),    //
                                     Types<f32V, AFloatV>(0.0_a, 0.0),                  //
                                     Types<f32V, AFloatV>(AFloat(kMaxF32), kMaxF32),    //
                                     Types<f32V, AFloatV>(AFloat(-kMaxF32), -kMaxF32),  //
                                     Types<f32V, AFloatV>(AFloat(kPiF32), kPiF64),      //
                                     Types<f32V, AFloatV>(0.0_a, kTooSmallF32),         //
                                     Types<f32V, AFloatV>(-0.0_a, -kTooSmallF32)        //
                                     /* Types<f16V, AFloatV>(1.0_a), */                 //
                                     /* Types<f16V, AFloatV>(1.0_a), */)));

INSTANTIATE_TEST_SUITE_P(
    MaterializeMatrix,
    MaterializeAbstractNumeric,                                                         //
    testing::Combine(testing::Values(Expectation::kMaterialize),                        //
                     testing::ValuesIn(kMatrixMethods),                                 //
                     testing::Values(Types<f32M, AFloatM>(0.0_a, 0.0),                  //
                                     Types<f32M, AFloatM>(AFloat(kMaxF32), kMaxF32),    //
                                     Types<f32M, AFloatM>(AFloat(-kMaxF32), -kMaxF32),  //
                                     Types<f32M, AFloatM>(AFloat(kPiF32), kPiF64),      //
                                     Types<f32M, AFloatM>(0.0_a, kTooSmallF32),         //
                                     Types<f32M, AFloatM>(-0.0_a, -kTooSmallF32)        //
                                     /* Types<f16V, AFloatM>(1.0_a), */                 //
                                     )));

INSTANTIATE_TEST_SUITE_P(
    MaterializeSwitch,
    MaterializeAbstractNumeric,                                                       //
    testing::Combine(testing::Values(Expectation::kMaterialize),                      //
                     testing::ValuesIn(kSwitchMethods),                               //
                     testing::Values(Types<i32, AInt>(0_a, 0.0),                      //
                                     Types<i32, AInt>(2147483647_a, 2147483647.0),    //
                                     Types<i32, AInt>(-2147483648_a, -2147483648.0),  //
                                     Types<u32, AInt>(0_a, 0.0),                      //
                                     Types<u32, AInt>(4294967295_a, 4294967295.0))));

// TODO(crbug.com/tint/1504): Enable once we have abstract overloads of builtins / binary ops.
INSTANTIATE_TEST_SUITE_P(DISABLED_NoMaterialize,
                         MaterializeAbstractNumeric,                                     //
                         testing::Combine(testing::Values(Expectation::kNoMaterialize),  //
                                          testing::Values(Method::kBuiltinArg,           //
                                                          Method::kBinaryOp),            //
                                          testing::Values(Types<AInt, AInt>(),           //
                                                          Types<AFloat, AFloat>(),       //
                                                          Types<AIntV, AIntV>(),         //
                                                          Types<AFloatV, AFloatV>(),     //
                                                          Types<AFloatM, AFloatM>())));
INSTANTIATE_TEST_SUITE_P(InvalidCast,
                         MaterializeAbstractNumeric,                                   //
                         testing::Combine(testing::Values(Expectation::kInvalidCast),  //
                                          testing::ValuesIn(kScalarMethods),           //
                                          testing::Values(Types<i32, AFloat>(),        //
                                                          Types<u32, AFloat>(),        //
                                                          Types<i32V, AFloatV>(),      //
                                                          Types<u32V, AFloatV>())));

INSTANTIATE_TEST_SUITE_P(
    ScalarValueCannotBeRepresented,
    MaterializeAbstractNumeric,                                                //
    testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),  //
                     testing::ValuesIn(kScalarMethods),                        //
                     testing::Values(Types<i32, AInt>(0_a, 2147483648.0),      //
                                     Types<i32, AInt>(0_a, -2147483649.0),     //
                                     Types<u32, AInt>(0_a, 4294967296),        //
                                     Types<u32, AInt>(0_a, -1.0),              //
                                     Types<f32, AFloat>(0.0_a, 3.5e+38),       //
                                     Types<f32, AFloat>(0.0_a, -3.5e+38)       //
                                     /* Types<f16, AFloat>(), */               //
                                     /* Types<f16, AFloat>(), */)));

INSTANTIATE_TEST_SUITE_P(
    VectorValueCannotBeRepresented,
    MaterializeAbstractNumeric,                                                //
    testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),  //
                     testing::ValuesIn(kVectorMethods),                        //
                     testing::Values(Types<i32V, AIntV>(0_a, 2147483648.0),    //
                                     Types<i32V, AIntV>(0_a, -2147483649.0),   //
                                     Types<u32V, AIntV>(0_a, 4294967296),      //
                                     Types<u32V, AIntV>(0_a, -1.0),            //
                                     Types<f32V, AFloatV>(0.0_a, 3.5e+38),     //
                                     Types<f32V, AFloatV>(0.0_a, -3.5e+38)     //
                                     /* Types<f16V, AFloatV>(), */             //
                                     /* Types<f16V, AFloatV>(), */)));

INSTANTIATE_TEST_SUITE_P(
    MatrixValueCannotBeRepresented,
    MaterializeAbstractNumeric,                                                //
    testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),  //
                     testing::ValuesIn(kMatrixMethods),                        //
                     testing::Values(Types<f32M, AFloatM>(0.0_a, 3.5e+38),     //
                                     Types<f32M, AFloatM>(0.0_a, -3.5e+38)     //
                                     /* Types<f16M, AFloatM>(), */             //
                                     /* Types<f16M, AFloatM>(), */)));

}  // namespace MaterializeTests

}  // namespace
}  // namespace tint::resolver
