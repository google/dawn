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
    kBuiltinFunction,
    kBuiltinType,
    kFunction,
    kStruct,
    kTypeAlias,
    kVariable,
};

std::ostream& operator<<(std::ostream& out, Def def) {
    switch (def) {
        case Def::kBuiltinFunction:
            return out << "Def::kBuiltinFunction";
        case Def::kBuiltinType:
            return out << "Def::kBuiltinType";
        case Def::kFunction:
            return out << "Def::kFunction";
        case Def::kStruct:
            return out << "Def::kStruct";
        case Def::kTypeAlias:
            return out << "Def::kTypeAlias";
        case Def::kVariable:
            return out << "Def::kVariable";
    }
    return out << "<unknown>";
}

enum class Use {
    kCallExpr,
    kCallStmt,
    kFunctionReturnType,
    kMemberType,
    kValueExpression,
    kVariableType,
};

std::ostream& operator<<(std::ostream& out, Use use) {
    switch (use) {
        case Use::kCallExpr:
            return out << "Use::kCallExpr";
        case Use::kCallStmt:
            return out << "Use::kCallStmt";
        case Use::kFunctionReturnType:
            return out << "Use::kFunctionReturnType";
        case Use::kMemberType:
            return out << "Use::kMemberType";
        case Use::kValueExpression:
            return out << "Use::kValueExpression";
        case Use::kVariableType:
            return out << "Use::kVariableType";
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
    switch (GetParam().def) {
        case Def::kBuiltinFunction:
            sym = Sym("workgroupBarrier");
            break;
        case Def::kBuiltinType:
            sym = Sym("vec4f");
            break;
        case Def::kFunction:
            Func(kDefSource, "FUNCTION", utils::Empty, ty.i32(), Return(1_i));
            sym = Sym("FUNCTION");
            break;
        case Def::kStruct:
            Structure(kDefSource, "STRUCT", utils::Vector{Member("m", ty.i32())});
            sym = Sym("STRUCT");
            break;
        case Def::kTypeAlias:
            Alias(kDefSource, "ALIAS", ty.i32());
            sym = Sym("ALIAS");
            break;
        case Def::kVariable:
            GlobalConst(kDefSource, "VARIABLE", Expr(1_i));
            sym = Sym("VARIABLE");
            break;
    }

    switch (GetParam().use) {
        case Use::kCallExpr:
            Func("f", utils::Empty, ty.void_(), Decl(Var("v", Call(Ident(kUseSource, sym)))));
            break;
        case Use::kCallStmt:
            Func("f", utils::Empty, ty.void_(), CallStmt(Call(Ident(kUseSource, sym))));
            break;
        case Use::kFunctionReturnType:
            Func("f", utils::Empty, ty(kUseSource, sym), Return(Call(sym)));
            break;
        case Use::kMemberType:
            Structure("s", utils::Vector{Member("m", ty(kUseSource, sym))});
            break;
        case Use::kValueExpression:
            GlobalVar("v", type::AddressSpace::kPrivate, Expr(kUseSource, sym));
            break;
        case Use::kVariableType:
            GlobalVar("v", type::AddressSpace::kPrivate, ty(kUseSource, sym));
            break;
    }

    if (GetParam().error == kPass) {
        EXPECT_TRUE(r()->Resolve());
        EXPECT_EQ(r()->error(), "");
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), GetParam().error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    ResolverExpressionKindTest,
    testing::ValuesIn(std::vector<Case>{
        {Def::kBuiltinFunction, Use::kCallStmt, kPass},
        {Def::kBuiltinFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},
        {Def::kBuiltinFunction, Use::kMemberType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},
        {Def::kBuiltinFunction, Use::kValueExpression,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kVariableType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},

        {Def::kBuiltinType, Use::kCallExpr, kPass},
        {Def::kBuiltinType, Use::kCallStmt, kPass},
        {Def::kBuiltinType, Use::kFunctionReturnType, kPass},
        {Def::kBuiltinType, Use::kMemberType, kPass},
        {Def::kBuiltinType, Use::kValueExpression,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kBuiltinType, Use::kVariableType, kPass},

        {Def::kFunction, Use::kCallExpr, kPass},
        {Def::kFunction, Use::kCallStmt, kPass},
        {Def::kFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kMemberType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kValueExpression, R"(7:8 error: missing '(' for function call)"},
        {Def::kFunction, Use::kVariableType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: 'FUNCTION' declared here)"},

        {Def::kStruct, Use::kCallExpr, kPass},
        {Def::kStruct, Use::kCallStmt, kPass},
        {Def::kStruct, Use::kFunctionReturnType, kPass},
        {Def::kStruct, Use::kMemberType, kPass},
        {Def::kStruct, Use::kValueExpression,
         R"(5:6 error: cannot use type 'STRUCT' as value
7:8 note: are you missing '()' for type initializer?
1:2 note: 'STRUCT' declared here)"},
        {Def::kStruct, Use::kVariableType, kPass},

        {Def::kTypeAlias, Use::kCallExpr, kPass},
        {Def::kTypeAlias, Use::kCallStmt, kPass},
        {Def::kTypeAlias, Use::kFunctionReturnType, kPass},
        {Def::kTypeAlias, Use::kMemberType, kPass},
        {Def::kTypeAlias, Use::kValueExpression,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kTypeAlias, Use::kVariableType, kPass},

        {Def::kVariable, Use::kCallStmt,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kCallExpr,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kFunctionReturnType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kMemberType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kValueExpression, kPass},
        {Def::kVariable, Use::kVariableType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: 'VARIABLE' declared here)"},
    }));

}  // namespace
}  // namespace tint::resolver
