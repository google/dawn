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

constexpr double kHighestU32 = static_cast<double>(u32::kHighest);
constexpr double kLowestU32 = static_cast<double>(u32::kLowest);
constexpr double kHighestI32 = static_cast<double>(i32::kHighest);
constexpr double kLowestI32 = static_cast<double>(i32::kLowest);
constexpr double kHighestF32 = static_cast<double>(f32::kHighest);
constexpr double kLowestF32 = static_cast<double>(f32::kLowest);
constexpr double kTooBigF32 = static_cast<double>(3.5e+38);
constexpr double kPiF64 = 3.141592653589793;
constexpr double kPiF32 = 3.1415927410125732;  // kPiF64 quantized to f32

constexpr double kSubnormalF32 = 0x1.0p-128;

enum class Expectation {
    kMaterialize,
    kNoMaterialize,
    kInvalidConversion,
    kValueCannotBeRepresented,
};

static std::ostream& operator<<(std::ostream& o, Expectation m) {
    switch (m) {
        case Expectation::kMaterialize:
            return o << "materialize";
        case Expectation::kNoMaterialize:
            return o << "no-materialize";
        case Expectation::kInvalidConversion:
            return o << "invalid-conversion";
        case Expectation::kValueCannotBeRepresented:
            return o << "value cannot be represented";
    }
    return o << "<unknown>";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MaterializeAbstractNumericToConcreteType
// Tests that an abstract-numeric will materialize to the expected concrete type
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace materialize_abstract_numeric_to_concrete_type {

// How should the materialization occur?
enum class Method {
    // var a : target_type = abstract_expr;
    kVar,

    // let a : target_type = abstract_expr;
    kLet,

    // var a : target_type;
    // a = abstract_expr;
    kAssign,

    // _ = abstract_expr;
    kPhonyAssign,

    // fn F(v : target_type) {}
    // fn x() {
    //   F(abstract_expr);
    // }
    kFnArg,

    // min(target_expr, abstract_expr);
    kBuiltinArg,

    // fn F() : target_type {
    //   return abstract_expr;
    // }
    kReturn,

    // array<target_type, 1>(abstract_expr);
    kArray,

    // struct S {
    //   v : target_type
    // };
    // fn x() {
    //   _ = S(abstract_expr)
    // }
    kStruct,

    // target_expr + abstract_expr
    kBinaryOp,

    // switch (abstract_expr) {
    //   case target_expr: {}
    //   default: {}
    // }
    kSwitchCond,

    // switch (target_expr) {
    //   case abstract_expr: {}
    //   default: {}
    // }
    kSwitchCase,

    // switch (abstract_expr) {
    //   case 123: {}
    //   case target_expr: {}
    //   default: {}
    // }
    kSwitchCondWithAbstractCase,

    // switch (target_expr) {
    //   case 123: {}
    //   case abstract_expr: {}
    //   default: {}
    // }
    kSwitchCaseWithAbstractCase,

    // @workgroup_size(target_expr, abstract_expr, 123)
    // @compute
    // fn f() {}
    kWorkgroupSize
};

static std::ostream& operator<<(std::ostream& o, Method m) {
    switch (m) {
        case Method::kVar:
            return o << "var";
        case Method::kLet:
            return o << "let";
        case Method::kAssign:
            return o << "assign";
        case Method::kPhonyAssign:
            return o << "phony-assign";
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
        case Method::kWorkgroupSize:
            return o << "workgroup-size";
    }
    return o << "<unknown>";
}

struct Data {
    std::string target_type_name;
    std::string target_element_type_name;
    builder::ast_type_func_ptr target_ast_ty;
    builder::sem_type_func_ptr target_sem_ty;
    builder::ast_expr_func_ptr target_expr;
    std::string abstract_type_name;
    builder::ast_expr_func_ptr abstract_expr;
    std::variant<AInt, AFloat> materialized_value;
    double literal_value;
};

template <typename TARGET_TYPE, typename ABSTRACT_TYPE, typename MATERIALIZED_TYPE>
Data Types(MATERIALIZED_TYPE materialized_value, double literal_value) {
    using TargetDataType = builder::DataType<TARGET_TYPE>;
    using AbstractDataType = builder::DataType<ABSTRACT_TYPE>;
    using TargetElementDataType = builder::DataType<typename TargetDataType::ElementType>;
    return {
        TargetDataType::Name(),         // target_type_name
        TargetElementDataType::Name(),  // target_element_type_name
        TargetDataType::AST,            // target_ast_ty
        TargetDataType::Sem,            // target_sem_ty
        TargetDataType::Expr,           // target_expr
        AbstractDataType::Name(),       // abstract_type_name
        AbstractDataType::Expr,         // abstract_expr
        materialized_value,
        literal_value,
    };
}

template <typename TARGET_TYPE, typename ABSTRACT_TYPE>
Data Types() {
    using TargetDataType = builder::DataType<TARGET_TYPE>;
    using AbstractDataType = builder::DataType<ABSTRACT_TYPE>;
    using TargetElementDataType = builder::DataType<typename TargetDataType::ElementType>;
    return {
        TargetDataType::Name(),         // target_type_name
        TargetElementDataType::Name(),  // target_element_type_name
        TargetDataType::AST,            // target_ast_ty
        TargetDataType::Sem,            // target_sem_ty
        TargetDataType::Expr,           // target_expr
        AbstractDataType::Name(),       // abstract_type_name
        AbstractDataType::Expr,         // abstract_expr
        0_a,
        0.0,
    };
}

static std::ostream& operator<<(std::ostream& o, const Data& c) {
    auto print_value = [&](auto&& v) { o << v; };
    o << "[" << c.target_type_name << " <- " << c.abstract_type_name << "] [";
    std::visit(print_value, c.materialized_value);
    o << " <- " << c.literal_value << "]";
    return o;
}

using MaterializeAbstractNumericToConcreteType =
    resolver::ResolverTestWithParam<std::tuple<Expectation, Method, Data>>;

TEST_P(MaterializeAbstractNumericToConcreteType, Test) {
    // Once F16 is properly supported, we'll need to enable this:
    // Enable(ast::Extension::kF16);

    const auto& param = GetParam();
    const auto& expectation = std::get<0>(param);
    const auto& method = std::get<1>(param);
    const auto& data = std::get<2>(param);

    auto target_ty = [&] { return data.target_ast_ty(*this); };
    auto target_expr = [&] { return data.target_expr(*this, 42); };
    auto* abstract_expr = data.abstract_expr(*this, data.literal_value);
    switch (method) {
        case Method::kVar:
            WrapInFunction(Decl(Var("a", target_ty(), abstract_expr)));
            break;
        case Method::kLet:
            WrapInFunction(Decl(Let("a", target_ty(), abstract_expr)));
            break;
        case Method::kAssign:
            WrapInFunction(Decl(Var("a", target_ty(), nullptr)), Assign("a", abstract_expr));
            break;
        case Method::kPhonyAssign:
            WrapInFunction(Assign(Phony(), abstract_expr));
            break;
        case Method::kFnArg:
            Func("F", {Param("P", target_ty())}, ty.void_(), {});
            WrapInFunction(CallStmt(Call("F", abstract_expr)));
            break;
        case Method::kBuiltinArg:
            WrapInFunction(CallStmt(Call("min", target_expr(), abstract_expr)));
            break;
        case Method::kReturn:
            Func("F", {}, target_ty(), {Return(abstract_expr)});
            break;
        case Method::kArray:
            WrapInFunction(Construct(ty.array(target_ty(), 1_i), abstract_expr));
            break;
        case Method::kStruct:
            Structure("S", {Member("v", target_ty())});
            WrapInFunction(Construct(ty.type_name("S"), abstract_expr));
            break;
        case Method::kBinaryOp:
            WrapInFunction(Add(target_expr(), abstract_expr));
            break;
        case Method::kSwitchCond:
            WrapInFunction(Switch(abstract_expr,                                         //
                                  Case(target_expr()->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCase:
            WrapInFunction(Switch(target_expr(),                                         //
                                  Case(abstract_expr->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCondWithAbstractCase:
            WrapInFunction(Switch(abstract_expr,                                         //
                                  Case(Expr(123_a)),                                     //
                                  Case(target_expr()->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kSwitchCaseWithAbstractCase:
            WrapInFunction(Switch(target_expr(),                                         //
                                  Case(Expr(123_a)),                                     //
                                  Case(abstract_expr->As<ast::IntLiteralExpression>()),  //
                                  DefaultCase()));
            break;
        case Method::kWorkgroupSize:
            Func("f", {}, ty.void_(), {},
                 {WorkgroupSize(target_expr(), abstract_expr, Expr(123_a)),
                  Stage(ast::PipelineStage::kCompute)});
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
            auto* materialize = Sem().Get<sem::Materialize>(abstract_expr);
            ASSERT_NE(materialize, nullptr);
            check_types_and_values(materialize);
            break;
        }
        case Expectation::kNoMaterialize: {
            ASSERT_TRUE(r()->Resolve()) << r()->error();
            auto* sem = Sem().Get(abstract_expr);
            ASSERT_NE(sem, nullptr);
            EXPECT_FALSE(sem->Is<sem::Materialize>());
            check_types_and_values(sem);
            break;
        }
        case Expectation::kInvalidConversion: {
            ASSERT_FALSE(r()->Resolve());
            std::string expect;
            switch (method) {
                case Method::kBuiltinArg:
                    expect = "error: no matching call to min(" + data.target_type_name + ", " +
                             data.abstract_type_name + ")";
                    break;
                case Method::kBinaryOp:
                    expect = "error: no matching overload for operator + (" +
                             data.target_type_name + ", " + data.abstract_type_name + ")";
                    break;
                default:
                    expect = "error: cannot convert value of type '" + data.abstract_type_name +
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
constexpr Method kScalarMethods[] = {
    Method::kLet,    Method::kVar,   Method::kAssign, Method::kFnArg,    Method::kBuiltinArg,
    Method::kReturn, Method::kArray, Method::kStruct, Method::kBinaryOp,
};

/// Methods that support vector materialization
constexpr Method kVectorMethods[] = {
    Method::kLet,    Method::kVar,   Method::kAssign, Method::kFnArg,    Method::kBuiltinArg,
    Method::kReturn, Method::kArray, Method::kStruct, Method::kBinaryOp,
};

/// Methods that support matrix materialization
constexpr Method kMatrixMethods[] = {
    Method::kLet,    Method::kVar,   Method::kAssign, Method::kFnArg,
    Method::kReturn, Method::kArray, Method::kStruct, Method::kBinaryOp,
};

/// Methods that support materialization for switch cases
constexpr Method kSwitchMethods[] = {
    Method::kSwitchCond,
    Method::kSwitchCase,
    Method::kSwitchCondWithAbstractCase,
    Method::kSwitchCaseWithAbstractCase,
};

/// Methods that do not materialize
constexpr Method kNoMaterializeMethods[] = {
    Method::kPhonyAssign,
    // TODO(crbug.com/tint/1504): Enable once we have abstract overloads of builtins / binary ops:
    // Method::kBuiltinArg, Method::kBinaryOp,
};
INSTANTIATE_TEST_SUITE_P(
    MaterializeScalar,
    MaterializeAbstractNumericToConcreteType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kScalarMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<i32, AInt>(0_a, 0.0),                                  //
                         Types<i32, AInt>(1_a, 1.0),                                  //
                         Types<i32, AInt>(-1_a, -1.0),                                //
                         Types<i32, AInt>(AInt(kHighestI32), kHighestI32),            //
                         Types<i32, AInt>(AInt(kLowestI32), kLowestI32),              //
                         Types<u32, AInt>(0_a, 0.0),                                  //
                         Types<u32, AInt>(1_a, 1.0),                                  //
                         Types<u32, AInt>(AInt(kHighestU32), kHighestU32),            //
                         Types<u32, AInt>(AInt(kLowestU32), kLowestU32),              //
                         Types<f32, AFloat>(0.0_a, 0.0),                              //
                         Types<f32, AFloat>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32, AFloat>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32, AFloat>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32, AFloat>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32, AFloat>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                         /* Types<f16, AFloat>(1.0_a), */                             //
                         /* Types<f16, AFloat>(1.0_a), */                             //
                     })));

INSTANTIATE_TEST_SUITE_P(
    MaterializeVector,
    MaterializeAbstractNumericToConcreteType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kVectorMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<i32V, AIntV>(0_a, 0.0),                                  //
                         Types<i32V, AIntV>(1_a, 1.0),                                  //
                         Types<i32V, AIntV>(-1_a, -1.0),                                //
                         Types<i32V, AIntV>(AInt(kHighestI32), kHighestI32),            //
                         Types<i32V, AIntV>(AInt(kLowestI32), kLowestI32),              //
                         Types<u32V, AIntV>(0_a, 0.0),                                  //
                         Types<u32V, AIntV>(1_a, 1.0),                                  //
                         Types<u32V, AIntV>(AInt(kHighestU32), kHighestU32),            //
                         Types<u32V, AIntV>(AInt(kLowestU32), kLowestU32),              //
                         Types<f32V, AFloatV>(0.0_a, 0.0),                              //
                         Types<f32V, AFloatV>(1.0_a, 1.0),                              //
                         Types<f32V, AFloatV>(-1.0_a, -1.0),                            //
                         Types<f32V, AFloatV>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32V, AFloatV>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32V, AFloatV>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32V, AFloatV>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32V, AFloatV>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                         /* Types<f16V, AFloatV>(1.0_a), */                             //
                         /* Types<f16V, AFloatV>(1.0_a), */                             //
                     })));

INSTANTIATE_TEST_SUITE_P(
    MaterializeMatrix,
    MaterializeAbstractNumericToConcreteType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kMatrixMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<f32M, AFloatM>(0.0_a, 0.0),                              //
                         Types<f32M, AFloatM>(1.0_a, 1.0),                              //
                         Types<f32M, AFloatM>(-1.0_a, -1.0),                            //
                         Types<f32M, AFloatM>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32M, AFloatM>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32M, AFloatM>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32M, AFloatM>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32M, AFloatM>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                         /* Types<f16V, AFloatM>(1.0_a), */                             //
                     })));

INSTANTIATE_TEST_SUITE_P(MaterializeSwitch,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kMaterialize),
                                          testing::ValuesIn(kSwitchMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, 0.0),                        //
                                              Types<i32, AInt>(1_a, 1.0),                        //
                                              Types<i32, AInt>(-1_a, -1.0),                      //
                                              Types<i32, AInt>(AInt(kHighestI32), kHighestI32),  //
                                              Types<i32, AInt>(AInt(kLowestI32), kLowestI32),    //
                                              Types<u32, AInt>(0_a, 0.0),                        //
                                              Types<u32, AInt>(1_a, 1.0),                        //
                                              Types<u32, AInt>(AInt(kHighestU32), kHighestU32),  //
                                              Types<u32, AInt>(AInt(kLowestU32), kLowestU32),    //
                                          })));

INSTANTIATE_TEST_SUITE_P(MaterializeWorkgroupSize,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kMaterialize),
                                          testing::Values(Method::kWorkgroupSize),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(1_a, 1.0),          //
                                              Types<i32, AInt>(10_a, 10.0),        //
                                              Types<i32, AInt>(65535_a, 65535.0),  //
                                              Types<u32, AInt>(1_a, 1.0),          //
                                              Types<u32, AInt>(10_a, 10.0),        //
                                              Types<u32, AInt>(65535_a, 65535.0),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(NoMaterialize,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kNoMaterialize),
                                          testing::ValuesIn(kNoMaterializeMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<AInt, AInt>(1_a, 1_a),            //
                                              Types<AIntV, AIntV>(1_a, 1_a),          //
                                              Types<AFloat, AFloat>(1.0_a, 1.0_a),    //
                                              Types<AFloatV, AFloatV>(1.0_a, 1.0_a),  //
                                              Types<AFloatM, AFloatM>(1.0_a, 1.0_a),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(InvalidConversion,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kInvalidConversion),
                                          testing::ValuesIn(kScalarMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AFloat>(),    //
                                              Types<u32, AFloat>(),    //
                                              Types<i32V, AFloatV>(),  //
                                              Types<u32V, AFloatV>(),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(ScalarValueCannotBeRepresented,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kScalarMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, kHighestI32 + 1),  //
                                              Types<i32, AInt>(0_a, kLowestI32 - 1),   //
                                              Types<u32, AInt>(0_a, kHighestU32 + 1),  //
                                              Types<u32, AInt>(0_a, kLowestU32 - 1),   //
                                              Types<f32, AFloat>(0.0_a, kTooBigF32),   //
                                              Types<f32, AFloat>(0.0_a, -kTooBigF32),  //
                                              /* Types<f16, AFloat>(), */              //
                                              /* Types<f16, AFloat>(), */              //
                                          })));

INSTANTIATE_TEST_SUITE_P(VectorValueCannotBeRepresented,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kVectorMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32V, AIntV>(0_a, kHighestI32 + 1),  //
                                              Types<i32V, AIntV>(0_a, kLowestI32 - 1),   //
                                              Types<u32V, AIntV>(0_a, kHighestU32 + 1),  //
                                              Types<u32V, AIntV>(0_a, kLowestU32 - 1),   //
                                              Types<f32V, AFloatV>(0.0_a, kTooBigF32),   //
                                              Types<f32V, AFloatV>(0.0_a, -kTooBigF32),  //
                                              /* Types<f16V, AFloatV>(), */              //
                                              /* Types<f16V, AFloatV>(), */              //
                                          })));

INSTANTIATE_TEST_SUITE_P(MatrixValueCannotBeRepresented,
                         MaterializeAbstractNumericToConcreteType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kMatrixMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<f32M, AFloatM>(0.0_a, kTooBigF32),   //
                                              Types<f32M, AFloatM>(0.0_a, -kTooBigF32),  //
                                              /* Types<f16M, AFloatM>(), */              //
                                              /* Types<f16M, AFloatM>(), */              //
                                          })));

}  // namespace materialize_abstract_numeric_to_concrete_type

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests that in the absence of a 'target type' an abstract-int will materialize to i32, and an
// abstract-float will materialize to f32.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace materialize_abstract_numeric_to_default_type {

// How should the materialization occur?
enum class Method {
    // var a = abstract_expr;
    kVar,

    // let a = abstract_expr;
    kLet,

    // min(abstract_expr, abstract_expr)
    kBuiltinArg,

    // bitcast<f32>(abstract_expr)
    kBitcastF32Arg,

    // bitcast<vec3<f32>>(abstract_expr)
    kBitcastVec3F32Arg,

    // array<i32, abstract_expr>()
    kArrayLength,

    // switch (abstract_expr) {
    //   case abstract_expr: {}
    //   default: {}
    // }
    kSwitch,

    // @workgroup_size(abstract_expr)
    // @compute
    // fn f() {}
    kWorkgroupSize,

    // arr[abstract_expr]
    kIndex,
};

static std::ostream& operator<<(std::ostream& o, Method m) {
    switch (m) {
        case Method::kVar:
            return o << "var";
        case Method::kLet:
            return o << "let";
        case Method::kBuiltinArg:
            return o << "builtin-arg";
        case Method::kBitcastF32Arg:
            return o << "bitcast-f32-arg";
        case Method::kBitcastVec3F32Arg:
            return o << "bitcast-vec3-f32-arg";
        case Method::kArrayLength:
            return o << "array-length";
        case Method::kSwitch:
            return o << "switch";
        case Method::kWorkgroupSize:
            return o << "workgroup-size";
        case Method::kIndex:
            return o << "index";
    }
    return o << "<unknown>";
}

struct Data {
    std::string expected_type_name;
    std::string expected_element_type_name;
    builder::sem_type_func_ptr expected_sem_ty;
    std::string abstract_type_name;
    builder::ast_expr_func_ptr abstract_expr;
    std::variant<AInt, AFloat> materialized_value;
    double literal_value;
};

template <typename EXPECTED_TYPE, typename ABSTRACT_TYPE, typename MATERIALIZED_TYPE>
Data Types(MATERIALIZED_TYPE materialized_value, double literal_value) {
    using ExpectedDataType = builder::DataType<EXPECTED_TYPE>;
    using AbstractDataType = builder::DataType<ABSTRACT_TYPE>;
    using TargetElementDataType = builder::DataType<typename ExpectedDataType::ElementType>;
    return {
        ExpectedDataType::Name(),       // expected_type_name
        TargetElementDataType::Name(),  // expected_element_type_name
        ExpectedDataType::Sem,          // expected_sem_ty
        AbstractDataType::Name(),       // abstract_type_name
        AbstractDataType::Expr,         // abstract_expr
        materialized_value,
        literal_value,
    };
}

static std::ostream& operator<<(std::ostream& o, const Data& c) {
    auto print_value = [&](auto&& v) { o << v; };
    o << "[" << c.expected_type_name << " <- " << c.abstract_type_name << "] [";
    std::visit(print_value, c.materialized_value);
    o << " <- " << c.literal_value << "]";
    return o;
}

using MaterializeAbstractNumericToDefaultType =
    resolver::ResolverTestWithParam<std::tuple<Expectation, Method, Data>>;

TEST_P(MaterializeAbstractNumericToDefaultType, Test) {
    // Once F16 is properly supported, we'll need to enable this:
    // Enable(ast::Extension::kF16);

    const auto& param = GetParam();
    const auto& expectation = std::get<0>(param);
    const auto& method = std::get<1>(param);
    const auto& data = std::get<2>(param);

    ast::ExpressionList abstract_exprs;
    auto abstract_expr = [&] {
        auto* expr = data.abstract_expr(*this, data.literal_value);
        abstract_exprs.emplace_back(expr);
        return expr;
    };
    switch (method) {
        case Method::kVar:
            WrapInFunction(Decl(Var("a", nullptr, abstract_expr())));
            break;
        case Method::kLet:
            WrapInFunction(Decl(Let("a", nullptr, abstract_expr())));
            break;
        case Method::kBuiltinArg:
            WrapInFunction(CallStmt(Call("min", abstract_expr(), abstract_expr())));
            break;
        case Method::kBitcastF32Arg:
            WrapInFunction(Bitcast<f32>(abstract_expr()));
            break;
        case Method::kBitcastVec3F32Arg:
            WrapInFunction(Bitcast(ty.vec3<f32>(), abstract_expr()));
            break;
        case Method::kArrayLength:
            WrapInFunction(Construct(ty.array(ty.i32(), abstract_expr())));
            break;
        case Method::kSwitch:
            WrapInFunction(Switch(abstract_expr(),
                                  Case(abstract_expr()->As<ast::IntLiteralExpression>()),
                                  DefaultCase()));
            break;
        case Method::kWorkgroupSize:
            Func("f", {}, ty.void_(), {},
                 {WorkgroupSize(abstract_expr()), Stage(ast::PipelineStage::kCompute)});
            break;
        case Method::kIndex:
            Global("arr", ty.array<i32, 4>(), ast::StorageClass::kPrivate);
            WrapInFunction(IndexAccessor("arr", abstract_expr()));
            break;
    }

    auto check_types_and_values = [&](const sem::Expression* expr) {
        auto* expected_sem_ty = data.expected_sem_ty(*this);

        EXPECT_TYPE(expr->Type(), expected_sem_ty);
        EXPECT_TYPE(expr->ConstantValue().Type(), expected_sem_ty);

        uint32_t num_elems = 0;
        const sem::Type* expected_sem_el_ty = sem::Type::ElementOf(expected_sem_ty, &num_elems);
        EXPECT_TYPE(expr->ConstantValue().ElementType(), expected_sem_el_ty);
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
            for (auto* expr : abstract_exprs) {
                auto* materialize = Sem().Get<sem::Materialize>(expr);
                ASSERT_NE(materialize, nullptr);
                check_types_and_values(materialize);
            }
            break;
        }
        case Expectation::kInvalidConversion: {
            ASSERT_FALSE(r()->Resolve());
            std::string expect;
            switch (method) {
                case Method::kBuiltinArg:
                    expect = "error: no matching call to min(" + data.abstract_type_name + ", " +
                             data.abstract_type_name + ")";
                    break;
                default:
                    expect = "error: cannot convert value of type '" + data.abstract_type_name +
                             "' to type '" + data.expected_type_name + "'";
                    break;
            }
            EXPECT_THAT(r()->error(), testing::StartsWith(expect));
            break;
        }
        case Expectation::kValueCannotBeRepresented:
            ASSERT_FALSE(r()->Resolve());
            EXPECT_THAT(r()->error(), testing::HasSubstr("cannot be represented as '" +
                                                         data.expected_element_type_name + "'"));
            break;
        default:
            FAIL() << "unhandled expectation: " << expectation;
    }
}

/// Methods that support scalar materialization
constexpr Method kScalarMethods[] = {
    Method::kLet,
    Method::kVar,
    Method::kBuiltinArg,
    Method::kBitcastF32Arg,
};

/// Methods that support abstract-integer materialization
/// Note: Doesn't contain kWorkgroupSize or kArrayLength as they have tighter constraints on the
///       range of allowed integer values.
constexpr Method kAIntMethods[] = {
    Method::kSwitch,
    Method::kIndex,
};

/// Methods that support vector materialization
constexpr Method kVectorMethods[] = {
    Method::kLet,
    Method::kVar,
    Method::kBuiltinArg,
    Method::kBitcastVec3F32Arg,
};

/// Methods that support matrix materialization
constexpr Method kMatrixMethods[] = {
    Method::kLet,
    Method::kVar,
};

INSTANTIATE_TEST_SUITE_P(
    MaterializeScalar,
    MaterializeAbstractNumericToDefaultType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kScalarMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<i32, AInt>(0_a, 0.0),                                  //
                         Types<i32, AInt>(1_a, 1.0),                                  //
                         Types<i32, AInt>(-1_a, -1.0),                                //
                         Types<i32, AInt>(AInt(kHighestI32), kHighestI32),            //
                         Types<i32, AInt>(AInt(kLowestI32), kLowestI32),              //
                         Types<f32, AFloat>(0.0_a, 0.0),                              //
                         Types<f32, AFloat>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32, AFloat>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32, AFloat>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32, AFloat>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32, AFloat>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                     })));

INSTANTIATE_TEST_SUITE_P(
    MaterializeVector,
    MaterializeAbstractNumericToDefaultType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kVectorMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<i32V, AIntV>(0_a, 0.0),                                  //
                         Types<i32V, AIntV>(1_a, 1.0),                                  //
                         Types<i32V, AIntV>(-1_a, -1.0),                                //
                         Types<i32V, AIntV>(AInt(kHighestI32), kHighestI32),            //
                         Types<i32V, AIntV>(AInt(kLowestI32), kLowestI32),              //
                         Types<f32V, AFloatV>(0.0_a, 0.0),                              //
                         Types<f32V, AFloatV>(1.0_a, 1.0),                              //
                         Types<f32V, AFloatV>(-1.0_a, -1.0),                            //
                         Types<f32V, AFloatV>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32V, AFloatV>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32V, AFloatV>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32V, AFloatV>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32V, AFloatV>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                     })));

INSTANTIATE_TEST_SUITE_P(
    MaterializeMatrix,
    MaterializeAbstractNumericToDefaultType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::ValuesIn(kMatrixMethods),
                     testing::ValuesIn(std::vector<Data>{
                         Types<f32M, AFloatM>(0.0_a, 0.0),                              //
                         Types<f32M, AFloatM>(1.0_a, 1.0),                              //
                         Types<f32M, AFloatM>(-1.0_a, -1.0),                            //
                         Types<f32M, AFloatM>(AFloat(kHighestF32), kHighestF32),        //
                         Types<f32M, AFloatM>(AFloat(kLowestF32), kLowestF32),          //
                         Types<f32M, AFloatM>(AFloat(kPiF32), kPiF64),                  //
                         Types<f32M, AFloatM>(AFloat(kSubnormalF32), kSubnormalF32),    //
                         Types<f32M, AFloatM>(AFloat(-kSubnormalF32), -kSubnormalF32),  //
                     })));

INSTANTIATE_TEST_SUITE_P(MaterializeAInt,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kMaterialize),
                                          testing::ValuesIn(kAIntMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, 0.0),                        //
                                              Types<i32, AInt>(10_a, 10.0),                      //
                                              Types<i32, AInt>(AInt(kHighestI32), kHighestI32),  //
                                              Types<i32, AInt>(AInt(kLowestI32), kLowestI32),    //
                                          })));

INSTANTIATE_TEST_SUITE_P(
    MaterializeArrayLength,
    MaterializeAbstractNumericToDefaultType,
    testing::Combine(testing::Values(Expectation::kMaterialize),
                     testing::Values(Method::kArrayLength),
                     testing::ValuesIn(std::vector<Data>{
                         Types<i32, AInt>(1_a, 1.0),        //
                         Types<i32, AInt>(10_a, 10.0),      //
                         Types<i32, AInt>(1000_a, 1000.0),  //
                         // Note: kHighestI32 cannot be used due to max-byte-size validation
                     })));

INSTANTIATE_TEST_SUITE_P(MaterializeWorkgroupSize,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kMaterialize),
                                          testing::Values(Method::kWorkgroupSize),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(1_a, 1.0),          //
                                              Types<i32, AInt>(10_a, 10.0),        //
                                              Types<i32, AInt>(65535_a, 65535.0),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(ScalarValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kScalarMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, kHighestI32 + 1),  //
                                              Types<i32, AInt>(0_a, kLowestI32 - 1),   //
                                              Types<f32, AFloat>(0.0_a, kTooBigF32),   //
                                              Types<f32, AFloat>(0.0_a, -kTooBigF32),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(VectorValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kVectorMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32V, AIntV>(0_a, kHighestI32 + 1),  //
                                              Types<i32V, AIntV>(0_a, kLowestI32 - 1),   //
                                              Types<i32V, AIntV>(0_a, kHighestU32 + 1),  //
                                              Types<f32V, AFloatV>(0.0_a, kTooBigF32),   //
                                              Types<f32V, AFloatV>(0.0_a, -kTooBigF32),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(MatrixValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kMatrixMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<f32M, AFloatM>(0.0_a, kTooBigF32),   //
                                              Types<f32M, AFloatM>(0.0_a, -kTooBigF32),  //
                                          })));

INSTANTIATE_TEST_SUITE_P(AIntValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::ValuesIn(kAIntMethods),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, kHighestI32 + 1),  //
                                              Types<i32, AInt>(0_a, kLowestI32 - 1),   //
                                          })));

INSTANTIATE_TEST_SUITE_P(WorkgroupSizeValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::Values(Method::kWorkgroupSize),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, kHighestI32 + 1),  //
                                              Types<i32, AInt>(0_a, kLowestI32 - 1),   //
                                          })));

INSTANTIATE_TEST_SUITE_P(ArrayLengthValueCannotBeRepresented,
                         MaterializeAbstractNumericToDefaultType,
                         testing::Combine(testing::Values(Expectation::kValueCannotBeRepresented),
                                          testing::Values(Method::kArrayLength),
                                          testing::ValuesIn(std::vector<Data>{
                                              Types<i32, AInt>(0_a, kHighestI32 + 1),  //
                                          })));

}  // namespace materialize_abstract_numeric_to_default_type

}  // namespace
}  // namespace tint::resolver
