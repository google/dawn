// Copyright 2023 The Tint Authors.
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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

enum class Def {
    kAccess,
    kAddressSpace,
    kBuiltinFunction,
    kBuiltinType,
    kBuiltinValue,
    kFunction,
    kInterpolationSampling,
    kInterpolationType,
    kParameter,
    kStruct,
    kTexelFormat,
    kTypeAlias,
    kVariable,
};

std::ostream& operator<<(std::ostream& out, Def def) {
    switch (def) {
        case Def::kAccess:
            return out << "Def::kAccess";
        case Def::kAddressSpace:
            return out << "Def::kAddressSpace";
        case Def::kBuiltinFunction:
            return out << "Def::kBuiltinFunction";
        case Def::kBuiltinType:
            return out << "Def::kBuiltinType";
        case Def::kBuiltinValue:
            return out << "Def::kBuiltinValue";
        case Def::kFunction:
            return out << "Def::kFunction";
        case Def::kInterpolationSampling:
            return out << "Def::kInterpolationSampling";
        case Def::kInterpolationType:
            return out << "Def::kInterpolationType";
        case Def::kParameter:
            return out << "Def::kParameter";
        case Def::kStruct:
            return out << "Def::kStruct";
        case Def::kTexelFormat:
            return out << "Def::kTexelFormat";
        case Def::kTypeAlias:
            return out << "Def::kTypeAlias";
        case Def::kVariable:
            return out << "Def::kVariable";
    }
    return out << "<unknown>";
}

enum class Use {
    kAccess,
    kAddressSpace,
    kBinaryOp,
    kBuiltinValue,
    kCallExpr,
    kCallStmt,
    kFunctionReturnType,
    kInterpolationSampling,
    kInterpolationType,
    kMemberType,
    kTexelFormat,
    kValueExpression,
    kVariableType,
    kUnaryOp
};

std::ostream& operator<<(std::ostream& out, Use use) {
    switch (use) {
        case Use::kAccess:
            return out << "Use::kAccess";
        case Use::kAddressSpace:
            return out << "Use::kAddressSpace";
        case Use::kBinaryOp:
            return out << "Use::kBinaryOp";
        case Use::kBuiltinValue:
            return out << "Use::kBuiltinValue";
        case Use::kCallExpr:
            return out << "Use::kCallExpr";
        case Use::kCallStmt:
            return out << "Use::kCallStmt";
        case Use::kFunctionReturnType:
            return out << "Use::kFunctionReturnType";
        case Use::kInterpolationSampling:
            return out << "Use::kInterpolationSampling";
        case Use::kInterpolationType:
            return out << "Use::kInterpolationType";
        case Use::kMemberType:
            return out << "Use::kMemberType";
        case Use::kTexelFormat:
            return out << "Use::kTexelFormat";
        case Use::kValueExpression:
            return out << "Use::kValueExpression";
        case Use::kVariableType:
            return out << "Use::kVariableType";
        case Use::kUnaryOp:
            return out << "Use::kUnaryOp";
    }
    return out << "<unknown>";
}

struct Case {
    Def def;
    Use use;
    const char* error;
};

std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "{" << c.def << ", " << c.use << "}";
}

static const char* kPass = "<pass>";

static const Source kDefSource{Source::Range{{1, 2}, {3, 4}}};
static const Source kUseSource{Source::Range{{5, 6}, {7, 8}}};

using ResolverExpressionKindTest = ResolverTestWithParam<Case>;

TEST_P(ResolverExpressionKindTest, Test) {
    Symbol sym;
    std::function<void(const sem::Expression*)> check_expr;

    utils::Vector<const ast::Parameter*, 2> fn_params;
    utils::Vector<const ast::Statement*, 2> fn_stmts;
    utils::Vector<const ast::Attribute*, 2> fn_attrs;

    switch (GetParam().def) {
        case Def::kAccess: {
            sym = Sym("write");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::Access>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::Access::kWrite);
            };
            break;
        }
        case Def::kAddressSpace: {
            sym = Sym("workgroup");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::AddressSpace>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::AddressSpace::kWorkgroup);
            };
            break;
        }
        case Def::kBuiltinFunction: {
            sym = Sym("workgroupBarrier");
            check_expr = [](const sem::Expression* expr) { EXPECT_EQ(expr, nullptr); };
            break;
        }
        case Def::kBuiltinType: {
            sym = Sym("vec4f");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<type::Vector>());
            };
            break;
        }
        case Def::kBuiltinValue: {
            sym = Sym("position");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::BuiltinValue>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::BuiltinValue::kPosition);
            };
            break;
        }
        case Def::kFunction: {
            sym = Sym("FUNCTION");
            auto* fn = Func(kDefSource, sym, utils::Empty, ty.i32(), Return(1_i));
            check_expr = [fn](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* fn_expr = expr->As<sem::FunctionExpression>();
                ASSERT_NE(fn_expr, nullptr);
                EXPECT_EQ(fn_expr->Function()->Declaration(), fn);
            };
            break;
        }
        case Def::kInterpolationSampling: {
            sym = Sym("center");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr =
                    expr->As<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::InterpolationSampling::kCenter);
            };
            break;
        }
        case Def::kInterpolationType: {
            sym = Sym("linear");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr =
                    expr->As<sem::BuiltinEnumExpression<builtin::InterpolationType>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::InterpolationType::kLinear);
            };
            break;
        }
        case Def::kParameter: {
            sym = Sym("PARAMETER");
            auto* param = Param(kDefSource, sym, ty.i32());
            fn_params.Push(param);
            check_expr = [param](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* user = expr->As<sem::VariableUser>();
                ASSERT_NE(user, nullptr);
                EXPECT_EQ(user->Variable()->Declaration(), param);
            };
            break;
        }
        case Def::kStruct: {
            sym = Sym("STRUCT");
            auto* s = Structure(kDefSource, sym, utils::Vector{Member("m", ty.i32())});
            check_expr = [s](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                auto* got = ty_expr->Type()->As<sem::Struct>();
                ASSERT_NE(got, nullptr);
                EXPECT_EQ(got->Declaration(), s);
            };
            break;
        }
        case Def::kTexelFormat: {
            sym = Sym("rgba8unorm");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::TexelFormat>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), builtin::TexelFormat::kRgba8Unorm);
            };
            break;
        }
        case Def::kTypeAlias: {
            sym = Sym("ALIAS");
            Alias(kDefSource, sym, ty.i32());
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<type::I32>());
            };
            break;
        }
        case Def::kVariable: {
            sym = Sym("VARIABLE");
            auto* c = GlobalConst(kDefSource, sym, Expr(1_i));
            check_expr = [c](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* var_expr = expr->As<sem::VariableUser>();
                ASSERT_NE(var_expr, nullptr);
                EXPECT_EQ(var_expr->Variable()->Declaration(), c);
            };
            break;
        }
    }

    auto* expr = Expr(Ident(kUseSource, sym));
    switch (GetParam().use) {
        case Use::kAccess:
            GlobalVar("v", ty("texture_storage_2d", "rgba8unorm", expr), Group(0_u), Binding(0_u));
            break;
        case Use::kAddressSpace:
            Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
            Func(Symbols().New(), utils::Vector{Param("p", ty("ptr", expr, ty.f32()))}, ty.void_(),
                 utils::Empty);
            break;
        case Use::kCallExpr:
            fn_stmts.Push(Decl(Var("v", Call(expr))));
            break;
        case Use::kCallStmt:
            fn_stmts.Push(CallStmt(Call(expr)));
            break;
        case Use::kBinaryOp:
            fn_stmts.Push(Decl(Var("v", Mul(1_a, expr))));
            break;
        case Use::kBuiltinValue:
            Func(Symbols().New(),
                 utils::Vector{Param("p", ty.vec4<f32>(), utils::Vector{Builtin(expr)})},
                 ty.void_(), utils::Empty, utils::Vector{Stage(ast::PipelineStage::kFragment)});
            break;
        case Use::kFunctionReturnType:
            Func(Symbols().New(), utils::Empty, ty(expr), Return(Call(sym)));
            break;
        case Use::kInterpolationSampling: {
            fn_params.Push(Param("p", ty.vec4<f32>(),
                                 utils::Vector{
                                     Location(0_a),
                                     Interpolate(builtin::InterpolationType::kLinear, expr),
                                 }));
            fn_attrs.Push(Stage(ast::PipelineStage::kFragment));
            break;
        }
        case Use::kInterpolationType: {
            fn_params.Push(Param("p", ty.vec4<f32>(),
                                 utils::Vector{
                                     Location(0_a),
                                     Interpolate(expr, builtin::InterpolationSampling::kCenter),
                                 }));
            fn_attrs.Push(Stage(ast::PipelineStage::kFragment));
            break;
        }
        case Use::kMemberType:
            Structure(Symbols().New(), utils::Vector{Member("m", ty(expr))});
            break;
        case Use::kTexelFormat:
            GlobalVar(Symbols().New(), ty("texture_storage_2d", ty(expr), "write"), Group(0_u),
                      Binding(0_u));
            break;
        case Use::kValueExpression:
            fn_stmts.Push(Decl(Var("v", expr)));
            break;
        case Use::kVariableType:
            fn_stmts.Push(Decl(Var("v", ty(expr))));
            break;
        case Use::kUnaryOp:
            fn_stmts.Push(Assign(Phony(), Negation(expr)));
            break;
    }

    if (!fn_params.IsEmpty() || !fn_stmts.IsEmpty()) {
        Func(Symbols().New(), std::move(fn_params), ty.void_(), std::move(fn_stmts),
             std::move(fn_attrs));
    }

    if (GetParam().error == kPass) {
        EXPECT_TRUE(r()->Resolve());
        EXPECT_EQ(r()->error(), "");
        check_expr(Sem().Get(expr));
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), GetParam().error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    ResolverExpressionKindTest,
    testing::ValuesIn(std::vector<Case>{
        {Def::kAccess, Use::kAccess, kPass},
        {Def::kAccess, Use::kAddressSpace,
         R"(5:6 error: cannot use access 'write' as address space)"},
        {Def::kAccess, Use::kBinaryOp, R"(5:6 error: cannot use access 'write' as value)"},
        {Def::kAccess, Use::kBuiltinValue,
         R"(5:6 error: cannot use access 'write' as builtin value)"},
        {Def::kAccess, Use::kCallExpr, R"(5:6 error: cannot use access 'write' as call target)"},
        {Def::kAccess, Use::kCallStmt, R"(5:6 error: cannot use access 'write' as call target)"},
        {Def::kAccess, Use::kFunctionReturnType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kInterpolationSampling,
         R"(5:6 error: cannot use access 'write' as interpolation sampling)"},
        {Def::kAccess, Use::kInterpolationType,
         R"(5:6 error: cannot use access 'write' as interpolation type)"},
        {Def::kAccess, Use::kMemberType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kTexelFormat,
         R"(5:6 error: cannot use access 'write' as texel format)"},
        {Def::kAccess, Use::kValueExpression, R"(5:6 error: cannot use access 'write' as value)"},
        {Def::kAccess, Use::kVariableType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kUnaryOp, R"(5:6 error: cannot use access 'write' as value)"},

        {Def::kAddressSpace, Use::kAccess,
         R"(5:6 error: cannot use address space 'workgroup' as access)"},
        {Def::kAddressSpace, Use::kAddressSpace, kPass},
        {Def::kAddressSpace, Use::kBinaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kBuiltinValue,
         R"(5:6 error: cannot use address space 'workgroup' as builtin value)"},
        {Def::kAddressSpace, Use::kCallExpr,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kCallStmt,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kFunctionReturnType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kInterpolationSampling,
         R"(5:6 error: cannot use address space 'workgroup' as interpolation sampling)"},
        {Def::kAddressSpace, Use::kInterpolationType,
         R"(5:6 error: cannot use address space 'workgroup' as interpolation type)"},
        {Def::kAddressSpace, Use::kMemberType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kTexelFormat,
         R"(5:6 error: cannot use address space 'workgroup' as texel format)"},
        {Def::kAddressSpace, Use::kValueExpression,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kVariableType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kUnaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},

        {Def::kBuiltinFunction, Use::kAccess,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kAddressSpace,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kBinaryOp,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kBuiltinValue,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kCallStmt, kPass},
        {Def::kBuiltinFunction, Use::kFunctionReturnType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kInterpolationSampling,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kInterpolationType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kMemberType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kTexelFormat,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kValueExpression,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kVariableType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kUnaryOp,
         R"(7:8 error: missing '(' for builtin function call)"},

        {Def::kBuiltinType, Use::kAccess, R"(5:6 error: cannot use type 'vec4<f32>' as access)"},
        {Def::kBuiltinType, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'vec4<f32>' as address space)"},
        {Def::kBuiltinType, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kBuiltinType, Use::kBuiltinValue,
         R"(5:6 error: cannot use type 'vec4<f32>' as builtin value)"},
        {Def::kBuiltinType, Use::kCallExpr, kPass},
        {Def::kBuiltinType, Use::kFunctionReturnType, kPass},
        {Def::kBuiltinType, Use::kInterpolationSampling,
         R"(5:6 error: cannot use type 'vec4<f32>' as interpolation sampling)"},
        {Def::kBuiltinType, Use::kInterpolationType,
         R"(5:6 error: cannot use type 'vec4<f32>' as interpolation type)"},
        {Def::kBuiltinType, Use::kMemberType, kPass},
        {Def::kBuiltinType, Use::kTexelFormat,
         R"(5:6 error: cannot use type 'vec4<f32>' as texel format)"},
        {Def::kBuiltinType, Use::kValueExpression,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kBuiltinType, Use::kVariableType, kPass},
        {Def::kBuiltinType, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for value constructor?)"},

        {Def::kBuiltinValue, Use::kAccess,
         R"(5:6 error: cannot use builtin value 'position' as access)"},
        {Def::kBuiltinValue, Use::kAddressSpace,
         R"(5:6 error: cannot use builtin value 'position' as address space)"},
        {Def::kBuiltinValue, Use::kBinaryOp,
         R"(5:6 error: cannot use builtin value 'position' as value)"},
        {Def::kBuiltinValue, Use::kBuiltinValue, kPass},
        {Def::kBuiltinValue, Use::kCallStmt,
         R"(5:6 error: cannot use builtin value 'position' as call target)"},
        {Def::kBuiltinValue, Use::kCallExpr,
         R"(5:6 error: cannot use builtin value 'position' as call target)"},
        {Def::kBuiltinValue, Use::kFunctionReturnType,
         R"(5:6 error: cannot use builtin value 'position' as type)"},
        {Def::kBuiltinValue, Use::kInterpolationSampling,
         R"(5:6 error: cannot use builtin value 'position' as interpolation sampling)"},
        {Def::kBuiltinValue, Use::kInterpolationType,
         R"(5:6 error: cannot use builtin value 'position' as interpolation type)"},
        {Def::kBuiltinValue, Use::kMemberType,
         R"(5:6 error: cannot use builtin value 'position' as type)"},
        {Def::kBuiltinValue, Use::kTexelFormat,
         R"(5:6 error: cannot use builtin value 'position' as texel format)"},
        {Def::kBuiltinValue, Use::kValueExpression,
         R"(5:6 error: cannot use builtin value 'position' as value)"},
        {Def::kBuiltinValue, Use::kVariableType,
         R"(5:6 error: cannot use builtin value 'position' as type)"},
        {Def::kBuiltinValue, Use::kUnaryOp,
         R"(5:6 error: cannot use builtin value 'position' as value)"},

        {Def::kFunction, Use::kAccess, R"(5:6 error: cannot use function 'FUNCTION' as access
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kAddressSpace,
         R"(5:6 error: cannot use function 'FUNCTION' as address space
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kBinaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kBuiltinValue,
         R"(5:6 error: cannot use function 'FUNCTION' as builtin value
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kCallExpr, kPass},
        {Def::kFunction, Use::kCallStmt, kPass},
        {Def::kFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kInterpolationSampling,
         R"(5:6 error: cannot use function 'FUNCTION' as interpolation sampling
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kInterpolationType,
         R"(5:6 error: cannot use function 'FUNCTION' as interpolation type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kMemberType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kTexelFormat,
         R"(5:6 error: cannot use function 'FUNCTION' as texel format
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kValueExpression,
         R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kVariableType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kUnaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},

        {Def::kInterpolationSampling, Use::kAccess,
         R"(5:6 error: cannot use interpolation sampling 'center' as access)"},
        {Def::kInterpolationSampling, Use::kAddressSpace,
         R"(5:6 error: cannot use interpolation sampling 'center' as address space)"},
        {Def::kInterpolationSampling, Use::kBinaryOp,
         R"(5:6 error: cannot use interpolation sampling 'center' as value)"},
        {Def::kInterpolationSampling, Use::kBuiltinValue,
         R"(5:6 error: cannot use interpolation sampling 'center' as builtin value)"},
        {Def::kInterpolationSampling, Use::kCallStmt,
         R"(5:6 error: cannot use interpolation sampling 'center' as call target)"},
        {Def::kInterpolationSampling, Use::kCallExpr,
         R"(5:6 error: cannot use interpolation sampling 'center' as call target)"},
        {Def::kInterpolationSampling, Use::kFunctionReturnType,
         R"(5:6 error: cannot use interpolation sampling 'center' as type)"},
        {Def::kInterpolationSampling, Use::kInterpolationSampling, kPass},
        {Def::kInterpolationSampling, Use::kInterpolationType,
         R"(5:6 error: cannot use interpolation sampling 'center' as interpolation type)"},
        {Def::kInterpolationSampling, Use::kMemberType,
         R"(5:6 error: cannot use interpolation sampling 'center' as type)"},
        {Def::kInterpolationSampling, Use::kTexelFormat,
         R"(5:6 error: cannot use interpolation sampling 'center' as texel format)"},
        {Def::kInterpolationSampling, Use::kValueExpression,
         R"(5:6 error: cannot use interpolation sampling 'center' as value)"},
        {Def::kInterpolationSampling, Use::kVariableType,
         R"(5:6 error: cannot use interpolation sampling 'center' as type)"},
        {Def::kInterpolationSampling, Use::kUnaryOp,
         R"(5:6 error: cannot use interpolation sampling 'center' as value)"},

        {Def::kInterpolationType, Use::kAccess,
         R"(5:6 error: cannot use interpolation type 'linear' as access)"},
        {Def::kInterpolationType, Use::kAddressSpace,
         R"(5:6 error: cannot use interpolation type 'linear' as address space)"},
        {Def::kInterpolationType, Use::kBinaryOp,
         R"(5:6 error: cannot use interpolation type 'linear' as value)"},
        {Def::kInterpolationType, Use::kBuiltinValue,
         R"(5:6 error: cannot use interpolation type 'linear' as builtin value)"},
        {Def::kInterpolationType, Use::kCallStmt,
         R"(5:6 error: cannot use interpolation type 'linear' as call target)"},
        {Def::kInterpolationType, Use::kCallExpr,
         R"(5:6 error: cannot use interpolation type 'linear' as call target)"},
        {Def::kInterpolationType, Use::kFunctionReturnType,
         R"(5:6 error: cannot use interpolation type 'linear' as type)"},
        {Def::kInterpolationType, Use::kInterpolationSampling,
         R"(5:6 error: cannot use interpolation type 'linear' as interpolation sampling)"},
        {Def::kInterpolationType, Use::kInterpolationType, kPass},
        {Def::kInterpolationType, Use::kMemberType,
         R"(5:6 error: cannot use interpolation type 'linear' as type)"},
        {Def::kInterpolationType, Use::kTexelFormat,
         R"(5:6 error: cannot use interpolation type 'linear' as texel format)"},
        {Def::kInterpolationType, Use::kValueExpression,
         R"(5:6 error: cannot use interpolation type 'linear' as value)"},
        {Def::kInterpolationType, Use::kVariableType,
         R"(5:6 error: cannot use interpolation type 'linear' as type)"},
        {Def::kInterpolationType, Use::kUnaryOp,
         R"(5:6 error: cannot use interpolation type 'linear' as value)"},

        {Def::kParameter, Use::kBinaryOp, kPass},
        {Def::kParameter, Use::kCallStmt,
         R"(5:6 error: cannot use parameter 'PARAMETER' as call target
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kCallExpr,
         R"(5:6 error: cannot use parameter 'PARAMETER' as call target
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kValueExpression, kPass},
        {Def::kParameter, Use::kVariableType,
         R"(5:6 error: cannot use parameter 'PARAMETER' as type
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kUnaryOp, kPass},

        {Def::kStruct, Use::kAccess, R"(5:6 error: cannot use type 'STRUCT' as access
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'STRUCT' as address space
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kBinaryOp, R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: struct 'STRUCT' declared here
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kStruct, Use::kBuiltinValue,
         R"(5:6 error: cannot use type 'STRUCT' as builtin value
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kFunctionReturnType, kPass},
        {Def::kStruct, Use::kInterpolationSampling,
         R"(5:6 error: cannot use type 'STRUCT' as interpolation sampling
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kInterpolationType,
         R"(5:6 error: cannot use type 'STRUCT' as interpolation type
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kMemberType, kPass},
        {Def::kStruct, Use::kTexelFormat, R"(5:6 error: cannot use type 'STRUCT' as texel format
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kValueExpression,
         R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: struct 'STRUCT' declared here
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kStruct, Use::kVariableType, kPass},
        {Def::kStruct, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: struct 'STRUCT' declared here
7:8 note: are you missing '()' for value constructor?)"},

        {Def::kTexelFormat, Use::kAccess,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as access)"},
        {Def::kTexelFormat, Use::kAddressSpace,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as address space)"},
        {Def::kTexelFormat, Use::kBinaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kBuiltinValue,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as builtin value)"},
        {Def::kTexelFormat, Use::kCallExpr,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kCallStmt,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kFunctionReturnType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kInterpolationSampling,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as interpolation sampling)"},
        {Def::kTexelFormat, Use::kInterpolationType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as interpolation type)"},
        {Def::kTexelFormat, Use::kMemberType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kTexelFormat, kPass},
        {Def::kTexelFormat, Use::kValueExpression,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kVariableType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kUnaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},

        {Def::kTypeAlias, Use::kAccess, R"(5:6 error: cannot use type 'i32' as access)"},
        {Def::kTypeAlias, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'i32' as address space)"},
        {Def::kTypeAlias, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kTypeAlias, Use::kBuiltinValue,
         R"(5:6 error: cannot use type 'i32' as builtin value)"},
        {Def::kTypeAlias, Use::kCallExpr, kPass},
        {Def::kTypeAlias, Use::kFunctionReturnType, kPass},
        {Def::kTypeAlias, Use::kInterpolationSampling,
         R"(5:6 error: cannot use type 'i32' as interpolation sampling)"},
        {Def::kTypeAlias, Use::kInterpolationType,
         R"(5:6 error: cannot use type 'i32' as interpolation type)"},
        {Def::kTypeAlias, Use::kMemberType, kPass},
        {Def::kTypeAlias, Use::kTexelFormat, R"(5:6 error: cannot use type 'i32' as texel format)"},
        {Def::kTypeAlias, Use::kValueExpression,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for value constructor?)"},
        {Def::kTypeAlias, Use::kVariableType, kPass},
        {Def::kTypeAlias, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for value constructor?)"},

        {Def::kVariable, Use::kAccess, R"(5:6 error: cannot use const 'VARIABLE' as access
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kAddressSpace,
         R"(5:6 error: cannot use const 'VARIABLE' as address space
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kBinaryOp, kPass},
        {Def::kVariable, Use::kBuiltinValue,
         R"(5:6 error: cannot use const 'VARIABLE' as builtin value
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kCallStmt,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kCallExpr,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kFunctionReturnType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kInterpolationSampling,
         R"(5:6 error: cannot use const 'VARIABLE' as interpolation sampling
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kInterpolationType,
         R"(5:6 error: cannot use const 'VARIABLE' as interpolation type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kMemberType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kTexelFormat,
         R"(5:6 error: cannot use const 'VARIABLE' as texel format
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kValueExpression, kPass},
        {Def::kVariable, Use::kVariableType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kUnaryOp, kPass},
    }));

}  // namespace
}  // namespace tint::resolver
