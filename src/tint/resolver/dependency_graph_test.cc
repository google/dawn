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

#include <string>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/resolver/resolver_test_helper.h"

namespace tint::resolver {
namespace {

using ::testing::ElementsAre;

template <typename T>
class ResolverDependencyGraphTestWithParam : public ResolverTestWithParam<T> {
  public:
    DependencyGraph Build(std::string expected_error = "") {
        DependencyGraph graph;
        auto result =
            DependencyGraph::Build(this->AST(), this->Symbols(), this->Diagnostics(), graph);
        if (expected_error.empty()) {
            EXPECT_TRUE(result) << this->Diagnostics().str();
        } else {
            EXPECT_FALSE(result);
            EXPECT_EQ(expected_error, this->Diagnostics().str());
        }
        return graph;
    }
};

using ResolverDependencyGraphTest = ResolverDependencyGraphTestWithParam<::testing::Test>;

////////////////////////////////////////////////////////////////////////////////
// Parameterized test helpers
////////////////////////////////////////////////////////////////////////////////

/// SymbolDeclKind is used by parameterized tests to enumerate the different
/// kinds of symbol declarations.
enum class SymbolDeclKind {
    GlobalVar,
    GlobalConst,
    Alias,
    Struct,
    Function,
    Parameter,
    LocalVar,
    LocalLet,
    NestedLocalVar,
    NestedLocalLet,
};

static constexpr SymbolDeclKind kAllSymbolDeclKinds[] = {
    SymbolDeclKind::GlobalVar,      SymbolDeclKind::GlobalConst, SymbolDeclKind::Alias,
    SymbolDeclKind::Struct,         SymbolDeclKind::Function,    SymbolDeclKind::Parameter,
    SymbolDeclKind::LocalVar,       SymbolDeclKind::LocalLet,    SymbolDeclKind::NestedLocalVar,
    SymbolDeclKind::NestedLocalLet,
};

static constexpr SymbolDeclKind kTypeDeclKinds[] = {
    SymbolDeclKind::Alias,
    SymbolDeclKind::Struct,
};

static constexpr SymbolDeclKind kValueDeclKinds[] = {
    SymbolDeclKind::GlobalVar,      SymbolDeclKind::GlobalConst, SymbolDeclKind::Parameter,
    SymbolDeclKind::LocalVar,       SymbolDeclKind::LocalLet,    SymbolDeclKind::NestedLocalVar,
    SymbolDeclKind::NestedLocalLet,
};

static constexpr SymbolDeclKind kGlobalDeclKinds[] = {
    SymbolDeclKind::GlobalVar, SymbolDeclKind::GlobalConst, SymbolDeclKind::Alias,
    SymbolDeclKind::Struct,    SymbolDeclKind::Function,
};

static constexpr SymbolDeclKind kLocalDeclKinds[] = {
    SymbolDeclKind::Parameter,      SymbolDeclKind::LocalVar,       SymbolDeclKind::LocalLet,
    SymbolDeclKind::NestedLocalVar, SymbolDeclKind::NestedLocalLet,
};

static constexpr SymbolDeclKind kGlobalValueDeclKinds[] = {
    SymbolDeclKind::GlobalVar,
    SymbolDeclKind::GlobalConst,
};

static constexpr SymbolDeclKind kFuncDeclKinds[] = {
    SymbolDeclKind::Function,
};

/// SymbolUseKind is used by parameterized tests to enumerate the different
/// kinds of symbol uses.
enum class SymbolUseKind {
    GlobalVarType,
    GlobalVarArrayElemType,
    GlobalVarArraySizeValue,
    GlobalVarVectorElemType,
    GlobalVarMatrixElemType,
    GlobalVarSampledTexElemType,
    GlobalVarMultisampledTexElemType,
    GlobalVarValue,
    GlobalLetType,
    GlobalLetArrayElemType,
    GlobalLetArraySizeValue,
    GlobalLetVectorElemType,
    GlobalLetMatrixElemType,
    GlobalLetValue,
    AliasType,
    StructMemberType,
    CallFunction,
    ParameterType,
    LocalVarType,
    LocalVarArrayElemType,
    LocalVarArraySizeValue,
    LocalVarVectorElemType,
    LocalVarMatrixElemType,
    LocalVarValue,
    LocalLetType,
    LocalLetValue,
    NestedLocalVarType,
    NestedLocalVarValue,
    NestedLocalLetType,
    NestedLocalLetValue,
    WorkgroupSizeValue,
};

static constexpr SymbolUseKind kTypeUseKinds[] = {
    SymbolUseKind::GlobalVarType,
    SymbolUseKind::GlobalVarArrayElemType,
    SymbolUseKind::GlobalVarArraySizeValue,
    SymbolUseKind::GlobalVarVectorElemType,
    SymbolUseKind::GlobalVarMatrixElemType,
    SymbolUseKind::GlobalVarSampledTexElemType,
    SymbolUseKind::GlobalVarMultisampledTexElemType,
    SymbolUseKind::GlobalLetType,
    SymbolUseKind::GlobalLetArrayElemType,
    SymbolUseKind::GlobalLetArraySizeValue,
    SymbolUseKind::GlobalLetVectorElemType,
    SymbolUseKind::GlobalLetMatrixElemType,
    SymbolUseKind::AliasType,
    SymbolUseKind::StructMemberType,
    SymbolUseKind::ParameterType,
    SymbolUseKind::LocalVarType,
    SymbolUseKind::LocalVarArrayElemType,
    SymbolUseKind::LocalVarArraySizeValue,
    SymbolUseKind::LocalVarVectorElemType,
    SymbolUseKind::LocalVarMatrixElemType,
    SymbolUseKind::LocalLetType,
    SymbolUseKind::NestedLocalVarType,
    SymbolUseKind::NestedLocalLetType,
};

static constexpr SymbolUseKind kValueUseKinds[] = {
    SymbolUseKind::GlobalVarValue,      SymbolUseKind::GlobalLetValue,
    SymbolUseKind::LocalVarValue,       SymbolUseKind::LocalLetValue,
    SymbolUseKind::NestedLocalVarValue, SymbolUseKind::NestedLocalLetValue,
    SymbolUseKind::WorkgroupSizeValue,
};

static constexpr SymbolUseKind kFuncUseKinds[] = {
    SymbolUseKind::CallFunction,
};

/// @returns the description of the symbol declaration kind.
/// @note: This differs from the strings used in diagnostic messages.
std::ostream& operator<<(std::ostream& out, SymbolDeclKind kind) {
    switch (kind) {
        case SymbolDeclKind::GlobalVar:
            return out << "global var";
        case SymbolDeclKind::GlobalConst:
            return out << "global let";
        case SymbolDeclKind::Alias:
            return out << "alias";
        case SymbolDeclKind::Struct:
            return out << "struct";
        case SymbolDeclKind::Function:
            return out << "function";
        case SymbolDeclKind::Parameter:
            return out << "parameter";
        case SymbolDeclKind::LocalVar:
            return out << "local var";
        case SymbolDeclKind::LocalLet:
            return out << "local let";
        case SymbolDeclKind::NestedLocalVar:
            return out << "nested local var";
        case SymbolDeclKind::NestedLocalLet:
            return out << "nested local let";
    }
    return out << "<unknown>";
}

/// @returns the description of the symbol use kind.
/// @note: This differs from the strings used in diagnostic messages.
std::ostream& operator<<(std::ostream& out, SymbolUseKind kind) {
    switch (kind) {
        case SymbolUseKind::GlobalVarType:
            return out << "global var type";
        case SymbolUseKind::GlobalVarValue:
            return out << "global var value";
        case SymbolUseKind::GlobalVarArrayElemType:
            return out << "global var array element type";
        case SymbolUseKind::GlobalVarArraySizeValue:
            return out << "global var array size value";
        case SymbolUseKind::GlobalVarVectorElemType:
            return out << "global var vector element type";
        case SymbolUseKind::GlobalVarMatrixElemType:
            return out << "global var matrix element type";
        case SymbolUseKind::GlobalVarSampledTexElemType:
            return out << "global var sampled_texture element type";
        case SymbolUseKind::GlobalVarMultisampledTexElemType:
            return out << "global var multisampled_texture element type";
        case SymbolUseKind::GlobalLetType:
            return out << "global let type";
        case SymbolUseKind::GlobalLetValue:
            return out << "global let value";
        case SymbolUseKind::GlobalLetArrayElemType:
            return out << "global let array element type";
        case SymbolUseKind::GlobalLetArraySizeValue:
            return out << "global let array size value";
        case SymbolUseKind::GlobalLetVectorElemType:
            return out << "global let vector element type";
        case SymbolUseKind::GlobalLetMatrixElemType:
            return out << "global let matrix element type";
        case SymbolUseKind::AliasType:
            return out << "alias type";
        case SymbolUseKind::StructMemberType:
            return out << "struct member type";
        case SymbolUseKind::CallFunction:
            return out << "call function";
        case SymbolUseKind::ParameterType:
            return out << "parameter type";
        case SymbolUseKind::LocalVarType:
            return out << "local var type";
        case SymbolUseKind::LocalVarArrayElemType:
            return out << "local var array element type";
        case SymbolUseKind::LocalVarArraySizeValue:
            return out << "local var array size value";
        case SymbolUseKind::LocalVarVectorElemType:
            return out << "local var vector element type";
        case SymbolUseKind::LocalVarMatrixElemType:
            return out << "local var matrix element type";
        case SymbolUseKind::LocalVarValue:
            return out << "local var value";
        case SymbolUseKind::LocalLetType:
            return out << "local let type";
        case SymbolUseKind::LocalLetValue:
            return out << "local let value";
        case SymbolUseKind::NestedLocalVarType:
            return out << "nested local var type";
        case SymbolUseKind::NestedLocalVarValue:
            return out << "nested local var value";
        case SymbolUseKind::NestedLocalLetType:
            return out << "nested local let type";
        case SymbolUseKind::NestedLocalLetValue:
            return out << "nested local let value";
        case SymbolUseKind::WorkgroupSizeValue:
            return out << "workgroup size value";
    }
    return out << "<unknown>";
}

/// @returns the the diagnostic message name used for the given use
std::string DiagString(SymbolUseKind kind) {
    switch (kind) {
        case SymbolUseKind::GlobalVarType:
        case SymbolUseKind::GlobalVarArrayElemType:
        case SymbolUseKind::GlobalVarVectorElemType:
        case SymbolUseKind::GlobalVarMatrixElemType:
        case SymbolUseKind::GlobalVarSampledTexElemType:
        case SymbolUseKind::GlobalVarMultisampledTexElemType:
        case SymbolUseKind::GlobalLetType:
        case SymbolUseKind::GlobalLetArrayElemType:
        case SymbolUseKind::GlobalLetVectorElemType:
        case SymbolUseKind::GlobalLetMatrixElemType:
        case SymbolUseKind::AliasType:
        case SymbolUseKind::StructMemberType:
        case SymbolUseKind::ParameterType:
        case SymbolUseKind::LocalVarType:
        case SymbolUseKind::LocalVarArrayElemType:
        case SymbolUseKind::LocalVarVectorElemType:
        case SymbolUseKind::LocalVarMatrixElemType:
        case SymbolUseKind::LocalLetType:
        case SymbolUseKind::NestedLocalVarType:
        case SymbolUseKind::NestedLocalLetType:
            return "type";
        case SymbolUseKind::GlobalVarValue:
        case SymbolUseKind::GlobalVarArraySizeValue:
        case SymbolUseKind::GlobalLetValue:
        case SymbolUseKind::GlobalLetArraySizeValue:
        case SymbolUseKind::LocalVarValue:
        case SymbolUseKind::LocalVarArraySizeValue:
        case SymbolUseKind::LocalLetValue:
        case SymbolUseKind::NestedLocalVarValue:
        case SymbolUseKind::NestedLocalLetValue:
        case SymbolUseKind::WorkgroupSizeValue:
            return "identifier";
        case SymbolUseKind::CallFunction:
            return "function";
    }
    return "<unknown>";
}

/// @returns the declaration scope depth for the symbol declaration kind.
///          Globals are at depth 0, parameters and locals are at depth 1,
///          nested locals are at depth 2.
int ScopeDepth(SymbolDeclKind kind) {
    switch (kind) {
        case SymbolDeclKind::GlobalVar:
        case SymbolDeclKind::GlobalConst:
        case SymbolDeclKind::Alias:
        case SymbolDeclKind::Struct:
        case SymbolDeclKind::Function:
            return 0;
        case SymbolDeclKind::Parameter:
        case SymbolDeclKind::LocalVar:
        case SymbolDeclKind::LocalLet:
            return 1;
        case SymbolDeclKind::NestedLocalVar:
        case SymbolDeclKind::NestedLocalLet:
            return 2;
    }
    return -1;
}

/// @returns the use depth for the symbol use kind.
///          Globals are at depth 0, parameters and locals are at depth 1,
///          nested locals are at depth 2.
int ScopeDepth(SymbolUseKind kind) {
    switch (kind) {
        case SymbolUseKind::GlobalVarType:
        case SymbolUseKind::GlobalVarValue:
        case SymbolUseKind::GlobalVarArrayElemType:
        case SymbolUseKind::GlobalVarArraySizeValue:
        case SymbolUseKind::GlobalVarVectorElemType:
        case SymbolUseKind::GlobalVarMatrixElemType:
        case SymbolUseKind::GlobalVarSampledTexElemType:
        case SymbolUseKind::GlobalVarMultisampledTexElemType:
        case SymbolUseKind::GlobalLetType:
        case SymbolUseKind::GlobalLetValue:
        case SymbolUseKind::GlobalLetArrayElemType:
        case SymbolUseKind::GlobalLetArraySizeValue:
        case SymbolUseKind::GlobalLetVectorElemType:
        case SymbolUseKind::GlobalLetMatrixElemType:
        case SymbolUseKind::AliasType:
        case SymbolUseKind::StructMemberType:
        case SymbolUseKind::WorkgroupSizeValue:
            return 0;
        case SymbolUseKind::CallFunction:
        case SymbolUseKind::ParameterType:
        case SymbolUseKind::LocalVarType:
        case SymbolUseKind::LocalVarArrayElemType:
        case SymbolUseKind::LocalVarArraySizeValue:
        case SymbolUseKind::LocalVarVectorElemType:
        case SymbolUseKind::LocalVarMatrixElemType:
        case SymbolUseKind::LocalVarValue:
        case SymbolUseKind::LocalLetType:
        case SymbolUseKind::LocalLetValue:
            return 1;
        case SymbolUseKind::NestedLocalVarType:
        case SymbolUseKind::NestedLocalVarValue:
        case SymbolUseKind::NestedLocalLetType:
        case SymbolUseKind::NestedLocalLetValue:
            return 2;
    }
    return -1;
}

/// A helper for building programs that exercise symbol declaration tests.
struct SymbolTestHelper {
    /// The program builder
    ProgramBuilder* const builder;
    /// Parameters to a function that may need to be built
    std::vector<const ast::Variable*> parameters;
    /// Shallow function var / let declaration statements
    std::vector<const ast::Statement*> statements;
    /// Nested function local var / let declaration statements
    std::vector<const ast::Statement*> nested_statements;
    /// Function attributes
    ast::AttributeList func_attrs;

    /// Constructor
    /// @param builder the program builder
    explicit SymbolTestHelper(ProgramBuilder* builder);

    /// Destructor.
    ~SymbolTestHelper();

    /// Declares a symbol with the given kind
    /// @param kind the kind of symbol declaration
    /// @param symbol the symbol to use for the declaration
    /// @param source the source of the declaration
    /// @returns the declaration node
    const ast::Node* Add(SymbolDeclKind kind, Symbol symbol, Source source);

    /// Declares a use of a symbol with the given kind
    /// @param kind the kind of symbol use
    /// @param symbol the declaration symbol to use
    /// @param source the source of the use
    /// @returns the use node
    const ast::Node* Add(SymbolUseKind kind, Symbol symbol, Source source);

    /// Builds a function, if any parameter or local declarations have been added
    void Build();
};

SymbolTestHelper::SymbolTestHelper(ProgramBuilder* b) : builder(b) {}

SymbolTestHelper::~SymbolTestHelper() {}

const ast::Node* SymbolTestHelper::Add(SymbolDeclKind kind, Symbol symbol, Source source) {
    auto& b = *builder;
    switch (kind) {
        case SymbolDeclKind::GlobalVar:
            return b.Global(source, symbol, b.ty.i32(), ast::StorageClass::kPrivate);
        case SymbolDeclKind::GlobalConst:
            return b.GlobalConst(source, symbol, b.ty.i32(), b.Expr(1));
        case SymbolDeclKind::Alias:
            return b.Alias(source, symbol, b.ty.i32());
        case SymbolDeclKind::Struct:
            return b.Structure(source, symbol, {b.Member("m", b.ty.i32())});
        case SymbolDeclKind::Function:
            return b.Func(source, symbol, {}, b.ty.void_(), {});
        case SymbolDeclKind::Parameter: {
            auto* node = b.Param(source, symbol, b.ty.i32());
            parameters.emplace_back(node);
            return node;
        }
        case SymbolDeclKind::LocalVar: {
            auto* node = b.Var(source, symbol, b.ty.i32());
            statements.emplace_back(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::LocalLet: {
            auto* node = b.Let(source, symbol, b.ty.i32(), b.Expr(1));
            statements.emplace_back(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::NestedLocalVar: {
            auto* node = b.Var(source, symbol, b.ty.i32());
            nested_statements.emplace_back(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::NestedLocalLet: {
            auto* node = b.Let(source, symbol, b.ty.i32(), b.Expr(1));
            nested_statements.emplace_back(b.Decl(node));
            return node;
        }
    }
    return nullptr;
}

const ast::Node* SymbolTestHelper::Add(SymbolUseKind kind, Symbol symbol, Source source) {
    auto& b = *builder;
    switch (kind) {
        case SymbolUseKind::GlobalVarType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), node, ast::StorageClass::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), b.ty.array(node, 4), ast::StorageClass::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            b.Global(b.Sym(), b.ty.array(b.ty.i32(), node), ast::StorageClass::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), b.ty.vec3(node), ast::StorageClass::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), b.ty.mat3x4(node), ast::StorageClass::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarSampledTexElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), b.ty.sampled_texture(ast::TextureDimension::k2d, node));
            return node;
        }
        case SymbolUseKind::GlobalVarMultisampledTexElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Global(b.Sym(), b.ty.multisampled_texture(ast::TextureDimension::k2d, node));
            return node;
        }
        case SymbolUseKind::GlobalVarValue: {
            auto* node = b.Expr(source, symbol);
            b.Global(b.Sym(), b.ty.i32(), ast::StorageClass::kPrivate, node);
            return node;
        }
        case SymbolUseKind::GlobalLetType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), node, b.Expr(1));
            return node;
        }
        case SymbolUseKind::GlobalLetArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.array(node, 4), b.Expr(1));
            return node;
        }
        case SymbolUseKind::GlobalLetArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.array(b.ty.i32(), node), b.Expr(1));
            return node;
        }
        case SymbolUseKind::GlobalLetVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.vec3(node), b.Expr(1));
            return node;
        }
        case SymbolUseKind::GlobalLetMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.mat3x4(node), b.Expr(1));
            return node;
        }
        case SymbolUseKind::GlobalLetValue: {
            auto* node = b.Expr(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.i32(), node);
            return node;
        }
        case SymbolUseKind::AliasType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Alias(b.Sym(), node);
            return node;
        }
        case SymbolUseKind::StructMemberType: {
            auto* node = b.ty.type_name(source, symbol);
            b.Structure(b.Sym(), {b.Member("m", node)});
            return node;
        }
        case SymbolUseKind::CallFunction: {
            auto* node = b.Expr(source, symbol);
            statements.emplace_back(b.CallStmt(b.Call(node)));
            return node;
        }
        case SymbolUseKind::ParameterType: {
            auto* node = b.ty.type_name(source, symbol);
            parameters.emplace_back(b.Param(b.Sym(), node));
            return node;
        }
        case SymbolUseKind::LocalVarType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.emplace_back(b.Decl(b.Var(b.Sym(), node)));
            return node;
        }
        case SymbolUseKind::LocalVarArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.emplace_back(b.Decl(b.Var(b.Sym(), b.ty.array(node, 4), b.Expr(1))));
            return node;
        }
        case SymbolUseKind::LocalVarArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            statements.emplace_back(
                b.Decl(b.Var(b.Sym(), b.ty.array(b.ty.i32(), node), b.Expr(1))));
            return node;
        }
        case SymbolUseKind::LocalVarVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.emplace_back(b.Decl(b.Var(b.Sym(), b.ty.vec3(node))));
            return node;
        }
        case SymbolUseKind::LocalVarMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.emplace_back(b.Decl(b.Var(b.Sym(), b.ty.mat3x4(node))));
            return node;
        }
        case SymbolUseKind::LocalVarValue: {
            auto* node = b.Expr(source, symbol);
            statements.emplace_back(b.Decl(b.Var(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::LocalLetType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.emplace_back(b.Decl(b.Let(b.Sym(), node, b.Expr(1))));
            return node;
        }
        case SymbolUseKind::LocalLetValue: {
            auto* node = b.Expr(source, symbol);
            statements.emplace_back(b.Decl(b.Let(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalVarType: {
            auto* node = b.ty.type_name(source, symbol);
            nested_statements.emplace_back(b.Decl(b.Var(b.Sym(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalVarValue: {
            auto* node = b.Expr(source, symbol);
            nested_statements.emplace_back(b.Decl(b.Var(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalLetType: {
            auto* node = b.ty.type_name(source, symbol);
            nested_statements.emplace_back(b.Decl(b.Let(b.Sym(), node, b.Expr(1))));
            return node;
        }
        case SymbolUseKind::NestedLocalLetValue: {
            auto* node = b.Expr(source, symbol);
            nested_statements.emplace_back(b.Decl(b.Let(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::WorkgroupSizeValue: {
            auto* node = b.Expr(source, symbol);
            func_attrs.emplace_back(b.WorkgroupSize(1, node, 2));
            return node;
        }
    }
    return nullptr;
}

void SymbolTestHelper::Build() {
    auto& b = *builder;
    if (!nested_statements.empty()) {
        statements.emplace_back(b.Block(nested_statements));
        nested_statements.clear();
    }
    if (!parameters.empty() || !statements.empty() || !func_attrs.empty()) {
        b.Func("func", parameters, b.ty.void_(), statements, func_attrs);
        parameters.clear();
        statements.clear();
        func_attrs.clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Used-before-declarated tests
////////////////////////////////////////////////////////////////////////////////
namespace used_before_decl_tests {

using ResolverDependencyGraphUsedBeforeDeclTest = ResolverDependencyGraphTest;

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, FuncCall) {
    // fn A() { B(); }
    // fn B() {}

    Func("A", {}, ty.void_(), {CallStmt(Call(Expr(Source{{12, 34}}, "B")))});
    Func(Source{{56, 78}}, "B", {}, ty.void_(), {Return()});

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeConstructed) {
    // fn F() {
    //   { _ = T(); }
    // }
    // type T = i32;

    Func("F", {}, ty.void_(), {Block(Ignore(Construct(ty.type_name(Source{{12, 34}}, "T"))))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedByLocal) {
    // fn F() {
    //   { var v : T; }
    // }
    // type T = i32;

    Func("F", {}, ty.void_(), {Block(Decl(Var("v", ty.type_name(Source{{12, 34}}, "T"))))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedByParam) {
    // fn F(p : T) {}
    // type T = i32;

    Func("F", {Param("p", ty.type_name(Source{{12, 34}}, "T"))}, ty.void_(), {});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedAsReturnType) {
    // fn F() -> T {}
    // type T = i32;

    Func("F", {}, ty.type_name(Source{{12, 34}}, "T"), {});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeByStructMember) {
    // struct S { m : T };
    // type T = i32;

    Structure("S", {Member("m", ty.type_name(Source{{12, 34}}, "T"))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, VarUsed) {
    // fn F() {
    //   { G = 3.14f; }
    // }
    // var G: f32 = 2.1;

    Func("F", ast::VariableList{}, ty.void_(), {Block(Assign(Expr(Source{{12, 34}}, "G"), 3.14f))});

    Global(Source{{56, 78}}, "G", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

    Build();
}

}  // namespace used_before_decl_tests

////////////////////////////////////////////////////////////////////////////////
// Undeclared symbol tests
////////////////////////////////////////////////////////////////////////////////
namespace undeclared_tests {

using ResolverDependencyGraphUndeclaredSymbolTest =
    ResolverDependencyGraphTestWithParam<SymbolUseKind>;

TEST_P(ResolverDependencyGraphUndeclaredSymbolTest, Test) {
    const Symbol symbol = Sym("SYMBOL");
    const auto use_kind = GetParam();

    // Build a use of a non-existent symbol
    SymbolTestHelper helper(this);
    helper.Add(use_kind, symbol, Source{{56, 78}});
    helper.Build();

    Build("56:78 error: unknown " + DiagString(use_kind) + ": 'SYMBOL'");
}

INSTANTIATE_TEST_SUITE_P(Types,
                         ResolverDependencyGraphUndeclaredSymbolTest,
                         testing::ValuesIn(kTypeUseKinds));

INSTANTIATE_TEST_SUITE_P(Values,
                         ResolverDependencyGraphUndeclaredSymbolTest,
                         testing::ValuesIn(kValueUseKinds));

INSTANTIATE_TEST_SUITE_P(Functions,
                         ResolverDependencyGraphUndeclaredSymbolTest,
                         testing::ValuesIn(kFuncUseKinds));

}  // namespace undeclared_tests

////////////////////////////////////////////////////////////////////////////////
// Self reference by decl
////////////////////////////////////////////////////////////////////////////////
namespace undeclared_tests {

using ResolverDependencyGraphDeclSelfUse = ResolverDependencyGraphTest;

TEST_F(ResolverDependencyGraphDeclSelfUse, GlobalVar) {
    const Symbol symbol = Sym("SYMBOL");
    Global(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123));
    Build(R"(error: cyclic dependency found: 'SYMBOL' -> 'SYMBOL'
12:34 note: var 'SYMBOL' references var 'SYMBOL' here)");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, GlobalConst) {
    const Symbol symbol = Sym("SYMBOL");
    GlobalConst(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123));
    Build(R"(error: cyclic dependency found: 'SYMBOL' -> 'SYMBOL'
12:34 note: let 'SYMBOL' references let 'SYMBOL' here)");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, LocalVar) {
    const Symbol symbol = Sym("SYMBOL");
    WrapInFunction(Decl(Var(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123))));
    Build("12:34 error: unknown identifier: 'SYMBOL'");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, LocalLet) {
    const Symbol symbol = Sym("SYMBOL");
    WrapInFunction(Decl(Let(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123))));
    Build("12:34 error: unknown identifier: 'SYMBOL'");
}

}  // namespace undeclared_tests

////////////////////////////////////////////////////////////////////////////////
// Recursive dependency tests
////////////////////////////////////////////////////////////////////////////////
namespace recursive_tests {

using ResolverDependencyGraphCyclicRefTest = ResolverDependencyGraphTest;

TEST_F(ResolverDependencyGraphCyclicRefTest, DirectCall) {
    // fn main() { main(); }

    Func(Source{{12, 34}}, "main", {}, ty.void_(),
         {CallStmt(Call(Expr(Source{{56, 78}}, "main")))});

    Build(R"(12:34 error: cyclic dependency found: 'main' -> 'main'
56:78 note: function 'main' calls function 'main' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, IndirectCall) {
    // 1: fn a() { b(); }
    // 2: fn e() { }
    // 3: fn d() { e(); b(); }
    // 4: fn c() { d(); }
    // 5: fn b() { c(); }

    Func(Source{{1, 1}}, "a", {}, ty.void_(), {CallStmt(Call(Expr(Source{{1, 10}}, "b")))});
    Func(Source{{2, 1}}, "e", {}, ty.void_(), {});
    Func(Source{{3, 1}}, "d", {}, ty.void_(),
         {
             CallStmt(Call(Expr(Source{{3, 10}}, "e"))),
             CallStmt(Call(Expr(Source{{3, 10}}, "b"))),
         });
    Func(Source{{4, 1}}, "c", {}, ty.void_(), {CallStmt(Call(Expr(Source{{4, 10}}, "d")))});
    Func(Source{{5, 1}}, "b", {}, ty.void_(), {CallStmt(Call(Expr(Source{{5, 10}}, "c")))});

    Build(R"(5:1 error: cyclic dependency found: 'b' -> 'c' -> 'd' -> 'b'
5:10 note: function 'b' calls function 'c' here
4:10 note: function 'c' calls function 'd' here
3:10 note: function 'd' calls function 'b' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Alias_Direct) {
    // type T = T;

    Alias(Source{{12, 34}}, "T", ty.type_name(Source{{56, 78}}, "T"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'T' -> 'T'
56:78 note: alias 'T' references alias 'T' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Alias_Indirect) {
    // 1: type Y = Z;
    // 2: type X = Y;
    // 3: type Z = X;

    Alias(Source{{1, 1}}, "Y", ty.type_name(Source{{1, 10}}, "Z"));
    Alias(Source{{2, 1}}, "X", ty.type_name(Source{{2, 10}}, "Y"));
    Alias(Source{{3, 1}}, "Z", ty.type_name(Source{{3, 10}}, "X"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: alias 'Y' references alias 'Z' here
3:10 note: alias 'Z' references alias 'X' here
2:10 note: alias 'X' references alias 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Struct_Direct) {
    // struct S {
    //   a: S;
    // };

    Structure(Source{{12, 34}}, "S", {Member("a", ty.type_name(Source{{56, 78}}, "S"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'S' -> 'S'
56:78 note: struct 'S' references struct 'S' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Struct_Indirect) {
    // 1: struct Y { z: Z; };
    // 2: struct X { y: Y; };
    // 3: struct Z { x: X; };

    Structure(Source{{1, 1}}, "Y", {Member("z", ty.type_name(Source{{1, 10}}, "Z"))});
    Structure(Source{{2, 1}}, "X", {Member("y", ty.type_name(Source{{2, 10}}, "Y"))});
    Structure(Source{{3, 1}}, "Z", {Member("x", ty.type_name(Source{{3, 10}}, "X"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: struct 'Y' references struct 'Z' here
3:10 note: struct 'Z' references struct 'X' here
2:10 note: struct 'X' references struct 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalVar_Direct) {
    // var<private> V : i32 = V;

    Global(Source{{12, 34}}, "V", ty.i32(), Expr(Source{{56, 78}}, "V"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'V' -> 'V'
56:78 note: var 'V' references var 'V' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalLet_Direct) {
    // let V : i32 = V;

    GlobalConst(Source{{12, 34}}, "V", ty.i32(), Expr(Source{{56, 78}}, "V"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'V' -> 'V'
56:78 note: let 'V' references let 'V' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalVar_Indirect) {
    // 1: var<private> Y : i32 = Z;
    // 2: var<private> X : i32 = Y;
    // 3: var<private> Z : i32 = X;

    Global(Source{{1, 1}}, "Y", ty.i32(), Expr(Source{{1, 10}}, "Z"));
    Global(Source{{2, 1}}, "X", ty.i32(), Expr(Source{{2, 10}}, "Y"));
    Global(Source{{3, 1}}, "Z", ty.i32(), Expr(Source{{3, 10}}, "X"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: var 'Y' references var 'Z' here
3:10 note: var 'Z' references var 'X' here
2:10 note: var 'X' references var 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalLet_Indirect) {
    // 1: let Y : i32 = Z;
    // 2: let X : i32 = Y;
    // 3: let Z : i32 = X;

    GlobalConst(Source{{1, 1}}, "Y", ty.i32(), Expr(Source{{1, 10}}, "Z"));
    GlobalConst(Source{{2, 1}}, "X", ty.i32(), Expr(Source{{2, 10}}, "Y"));
    GlobalConst(Source{{3, 1}}, "Z", ty.i32(), Expr(Source{{3, 10}}, "X"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: let 'Y' references let 'Z' here
3:10 note: let 'Z' references let 'X' here
2:10 note: let 'X' references let 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Mixed_RecursiveDependencies) {
    // 1: fn F() -> R { return Z; }
    // 2: type A = S;
    // 3: struct S { a : A };
    // 4: var Z = L;
    // 5: type R = A;
    // 6: let L : S = Z;

    Func(Source{{1, 1}}, "F", {}, ty.type_name(Source{{1, 5}}, "R"),
         {Return(Expr(Source{{1, 10}}, "Z"))});
    Alias(Source{{2, 1}}, "A", ty.type_name(Source{{2, 10}}, "S"));
    Structure(Source{{3, 1}}, "S", {Member("a", ty.type_name(Source{{3, 10}}, "A"))});
    Global(Source{{4, 1}}, "Z", nullptr, Expr(Source{{4, 10}}, "L"));
    Alias(Source{{5, 1}}, "R", ty.type_name(Source{{5, 10}}, "A"));
    GlobalConst(Source{{6, 1}}, "L", ty.type_name(Source{{5, 5}}, "S"), Expr(Source{{5, 10}}, "Z"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(2:1 error: cyclic dependency found: 'A' -> 'S' -> 'A'
2:10 note: alias 'A' references struct 'S' here
3:10 note: struct 'S' references alias 'A' here
4:1 error: cyclic dependency found: 'Z' -> 'L' -> 'Z'
4:10 note: var 'Z' references let 'L' here
5:10 note: let 'L' references var 'Z' here)");
}

}  // namespace recursive_tests

////////////////////////////////////////////////////////////////////////////////
// Symbol Redeclaration tests
////////////////////////////////////////////////////////////////////////////////
namespace redeclaration_tests {

using ResolverDependencyGraphRedeclarationTest =
    ResolverDependencyGraphTestWithParam<std::tuple<SymbolDeclKind, SymbolDeclKind>>;

TEST_P(ResolverDependencyGraphRedeclarationTest, Test) {
    const auto symbol = Sym("SYMBOL");

    auto a_kind = std::get<0>(GetParam());
    auto b_kind = std::get<1>(GetParam());

    auto a_source = Source{{12, 34}};
    auto b_source = Source{{56, 78}};

    if (a_kind != SymbolDeclKind::Parameter && b_kind == SymbolDeclKind::Parameter) {
        std::swap(a_source, b_source);  // Parameters are declared before locals
    }

    SymbolTestHelper helper(this);
    helper.Add(a_kind, symbol, a_source);
    helper.Add(b_kind, symbol, b_source);
    helper.Build();

    bool error = ScopeDepth(a_kind) == ScopeDepth(b_kind);

    Build(error ? R"(56:78 error: redeclaration of 'SYMBOL'
12:34 note: 'SYMBOL' previously declared here)"
                : "");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverDependencyGraphRedeclarationTest,
                         testing::Combine(testing::ValuesIn(kAllSymbolDeclKinds),
                                          testing::ValuesIn(kAllSymbolDeclKinds)));

}  // namespace redeclaration_tests

////////////////////////////////////////////////////////////////////////////////
// Ordered global tests
////////////////////////////////////////////////////////////////////////////////
namespace ordered_globals {

using ResolverDependencyGraphOrderedGlobalsTest =
    ResolverDependencyGraphTestWithParam<std::tuple<SymbolDeclKind, SymbolUseKind>>;

TEST_P(ResolverDependencyGraphOrderedGlobalsTest, InOrder) {
    const Symbol symbol = Sym("SYMBOL");
    const auto decl_kind = std::get<0>(GetParam());
    const auto use_kind = std::get<1>(GetParam());

    // Declaration before use
    SymbolTestHelper helper(this);
    helper.Add(decl_kind, symbol, Source{{12, 34}});
    helper.Add(use_kind, symbol, Source{{56, 78}});
    helper.Build();

    ASSERT_EQ(AST().GlobalDeclarations().size(), 2u);

    auto* decl = AST().GlobalDeclarations()[0];
    auto* use = AST().GlobalDeclarations()[1];
    EXPECT_THAT(Build().ordered_globals, ElementsAre(decl, use));
}

TEST_P(ResolverDependencyGraphOrderedGlobalsTest, OutOfOrder) {
    const Symbol symbol = Sym("SYMBOL");
    const auto decl_kind = std::get<0>(GetParam());
    const auto use_kind = std::get<1>(GetParam());

    // Use before declaration
    SymbolTestHelper helper(this);
    helper.Add(use_kind, symbol, Source{{56, 78}});
    helper.Build();  // If the use is in a function, then ensure this function is
                     // built before the symbol declaration
    helper.Add(decl_kind, symbol, Source{{12, 34}});
    helper.Build();

    ASSERT_EQ(AST().GlobalDeclarations().size(), 2u);

    auto* use = AST().GlobalDeclarations()[0];
    auto* decl = AST().GlobalDeclarations()[1];
    EXPECT_THAT(Build().ordered_globals, ElementsAre(decl, use));
}

INSTANTIATE_TEST_SUITE_P(Types,
                         ResolverDependencyGraphOrderedGlobalsTest,
                         testing::Combine(testing::ValuesIn(kTypeDeclKinds),
                                          testing::ValuesIn(kTypeUseKinds)));

INSTANTIATE_TEST_SUITE_P(Values,
                         ResolverDependencyGraphOrderedGlobalsTest,
                         testing::Combine(testing::ValuesIn(kGlobalValueDeclKinds),
                                          testing::ValuesIn(kValueUseKinds)));

INSTANTIATE_TEST_SUITE_P(Functions,
                         ResolverDependencyGraphOrderedGlobalsTest,
                         testing::Combine(testing::ValuesIn(kFuncDeclKinds),
                                          testing::ValuesIn(kFuncUseKinds)));
}  // namespace ordered_globals

////////////////////////////////////////////////////////////////////////////////
// Resolved symbols tests
////////////////////////////////////////////////////////////////////////////////
namespace resolved_symbols {

using ResolverDependencyGraphResolvedSymbolTest =
    ResolverDependencyGraphTestWithParam<std::tuple<SymbolDeclKind, SymbolUseKind>>;

TEST_P(ResolverDependencyGraphResolvedSymbolTest, Test) {
    const Symbol symbol = Sym("SYMBOL");
    const auto decl_kind = std::get<0>(GetParam());
    const auto use_kind = std::get<1>(GetParam());

    // Build a symbol declaration and a use of that symbol
    SymbolTestHelper helper(this);
    auto* decl = helper.Add(decl_kind, symbol, Source{{12, 34}});
    auto* use = helper.Add(use_kind, symbol, Source{{56, 78}});
    helper.Build();

    // If the declaration is visible to the use, then we expect the analysis to
    // succeed.
    bool expect_pass = ScopeDepth(decl_kind) <= ScopeDepth(use_kind);
    auto graph = Build(expect_pass ? "" : "56:78 error: unknown identifier: 'SYMBOL'");

    if (expect_pass) {
        // Check that the use resolves to the declaration
        auto* resolved_symbol = graph.resolved_symbols[use];
        EXPECT_EQ(resolved_symbol, decl)
            << "resolved: " << (resolved_symbol ? resolved_symbol->TypeInfo().name : "<null>")
            << "\n"
            << "decl:     " << decl->TypeInfo().name;
    }
}

INSTANTIATE_TEST_SUITE_P(Types,
                         ResolverDependencyGraphResolvedSymbolTest,
                         testing::Combine(testing::ValuesIn(kTypeDeclKinds),
                                          testing::ValuesIn(kTypeUseKinds)));

INSTANTIATE_TEST_SUITE_P(Values,
                         ResolverDependencyGraphResolvedSymbolTest,
                         testing::Combine(testing::ValuesIn(kValueDeclKinds),
                                          testing::ValuesIn(kValueUseKinds)));

INSTANTIATE_TEST_SUITE_P(Functions,
                         ResolverDependencyGraphResolvedSymbolTest,
                         testing::Combine(testing::ValuesIn(kFuncDeclKinds),
                                          testing::ValuesIn(kFuncUseKinds)));

}  // namespace resolved_symbols

////////////////////////////////////////////////////////////////////////////////
// Shadowing tests
////////////////////////////////////////////////////////////////////////////////
namespace shadowing {

using ResolverDependencyShadowTest =
    ResolverDependencyGraphTestWithParam<std::tuple<SymbolDeclKind, SymbolDeclKind>>;

TEST_P(ResolverDependencyShadowTest, Test) {
    const Symbol symbol = Sym("SYMBOL");
    const auto outer_kind = std::get<0>(GetParam());
    const auto inner_kind = std::get<1>(GetParam());

    // Build a symbol declaration and a use of that symbol
    SymbolTestHelper helper(this);
    auto* outer = helper.Add(outer_kind, symbol, Source{{12, 34}});
    helper.Add(inner_kind, symbol, Source{{56, 78}});
    auto* inner_var = helper.nested_statements.size()
                          ? helper.nested_statements[0]->As<ast::VariableDeclStatement>()->variable
                      : helper.statements.size()
                          ? helper.statements[0]->As<ast::VariableDeclStatement>()->variable
                          : helper.parameters[0];
    helper.Build();

    EXPECT_EQ(Build().shadows[inner_var], outer);
}

INSTANTIATE_TEST_SUITE_P(LocalShadowGlobal,
                         ResolverDependencyShadowTest,
                         testing::Combine(testing::ValuesIn(kGlobalDeclKinds),
                                          testing::ValuesIn(kLocalDeclKinds)));

INSTANTIATE_TEST_SUITE_P(NestedLocalShadowLocal,
                         ResolverDependencyShadowTest,
                         testing::Combine(testing::Values(SymbolDeclKind::Parameter,
                                                          SymbolDeclKind::LocalVar,
                                                          SymbolDeclKind::LocalLet),
                                          testing::Values(SymbolDeclKind::NestedLocalVar,
                                                          SymbolDeclKind::NestedLocalLet)));

}  // namespace shadowing

////////////////////////////////////////////////////////////////////////////////
// AST traversal tests
////////////////////////////////////////////////////////////////////////////////
namespace ast_traversal {

using ResolverDependencyGraphTraversalTest = ResolverDependencyGraphTest;

TEST_F(ResolverDependencyGraphTraversalTest, SymbolsReached) {
    const auto value_sym = Sym("VALUE");
    const auto type_sym = Sym("TYPE");
    const auto func_sym = Sym("FUNC");

    const auto* value_decl = Global(value_sym, ty.i32(), ast::StorageClass::kPrivate);
    const auto* type_decl = Alias(type_sym, ty.i32());
    const auto* func_decl = Func(func_sym, {}, ty.void_(), {});

    struct SymbolUse {
        const ast::Node* decl = nullptr;
        const ast::Node* use = nullptr;
        std::string where;
    };

    std::vector<SymbolUse> symbol_uses;

    auto add_use = [&](const ast::Node* decl, auto* use, int line, const char* kind) {
        symbol_uses.emplace_back(
            SymbolUse{decl, use, std::string(__FILE__) + ":" + std::to_string(line) + ": " + kind});
        return use;
    };
#define V add_use(value_decl, Expr(value_sym), __LINE__, "V()")
#define T add_use(type_decl, ty.type_name(type_sym), __LINE__, "T()")
#define F add_use(func_decl, Expr(func_sym), __LINE__, "F()")

    Alias(Sym(), T);
    Structure(Sym(), {Member(Sym(), T)});
    Global(Sym(), T, V);
    GlobalConst(Sym(), T, V);
    Func(Sym(),              //
         {Param(Sym(), T)},  //
         T,                  // Return type
         {
             Decl(Var(Sym(), T, V)),                    //
             Decl(Let(Sym(), T, V)),                    //
             CallStmt(Call(F, V)),                      //
             Block(                                     //
                 Assign(V, V)),                         //
             If(V,                                      //
                Block(Assign(V, V)),                    //
                Else(If(V,                              //
                        Block(Assign(V, V))))),         //
             Ignore(Bitcast(T, V)),                     //
             For(Decl(Var(Sym(), T, V)),                //
                 Equal(V, V),                           //
                 Assign(V, V),                          //
                 Block(                                 //
                     Assign(V, V))),                    //
             Loop(Block(Assign(V, V)),                  //
                  Block(Assign(V, V))),                 //
             Switch(V,                                  //
                    Case(Expr(1),                       //
                         Block(Assign(V, V))),          //
                    Case(Expr(2),                       //
                         Block(Fallthrough())),         //
                    DefaultCase(Block(Assign(V, V)))),  //
             Return(V),                                 //
             Break(),                                   //
             Discard(),                                 //
         });                                            //
    // Exercise type traversal
    Global(Sym(), ty.atomic(T));
    Global(Sym(), ty.bool_());
    Global(Sym(), ty.i32());
    Global(Sym(), ty.u32());
    Global(Sym(), ty.f32());
    Global(Sym(), ty.array(T, V, 4));
    Global(Sym(), ty.vec3(T));
    Global(Sym(), ty.mat3x2(T));
    Global(Sym(), ty.pointer(T, ast::StorageClass::kPrivate));
    Global(Sym(), ty.sampled_texture(ast::TextureDimension::k2d, T));
    Global(Sym(), ty.depth_texture(ast::TextureDimension::k2d));
    Global(Sym(), ty.depth_multisampled_texture(ast::TextureDimension::k2d));
    Global(Sym(), ty.external_texture());
    Global(Sym(), ty.multisampled_texture(ast::TextureDimension::k2d, T));
    Global(Sym(), ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                     ast::Access::kRead));  //
    Global(Sym(), ty.sampler(ast::SamplerKind::kSampler));
    Func(Sym(), {}, ty.void_(), {});
#undef V
#undef T
#undef F

    auto graph = Build();
    for (auto use : symbol_uses) {
        auto* resolved_symbol = graph.resolved_symbols[use.use];
        EXPECT_EQ(resolved_symbol, use.decl) << use.where;
    }
}

TEST_F(ResolverDependencyGraphTraversalTest, InferredType) {
    // Check that the nullptr of the var / let type doesn't make things explode
    Global("a", nullptr, Expr(1));
    GlobalConst("b", nullptr, Expr(1));
    WrapInFunction(Var("c", nullptr, Expr(1)),  //
                   Let("d", nullptr, Expr(1)));
    Build();
}

// Reproduces an unbalanced stack push / pop bug in
// DependencyAnalysis::SortGlobals(), found by clusterfuzz.
// See: crbug.com/chromium/1273451
TEST_F(ResolverDependencyGraphTraversalTest, chromium_1273451) {
    Structure("A", {Member("a", ty.i32())});
    Structure("B", {Member("b", ty.i32())});
    Func("f", {Param("a", ty.type_name("A"))}, ty.type_name("B"),
         {
             Return(Construct(ty.type_name("B"))),
         });
    Build();
}

}  // namespace ast_traversal

}  // namespace
}  // namespace tint::resolver
