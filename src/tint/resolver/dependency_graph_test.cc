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

using namespace tint::number_suffixes;  // NOLINT

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
    GlobalConstType,
    GlobalConstArrayElemType,
    GlobalConstArraySizeValue,
    GlobalConstVectorElemType,
    GlobalConstMatrixElemType,
    GlobalConstValue,
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
    SymbolUseKind::GlobalConstType,
    SymbolUseKind::GlobalConstArrayElemType,
    SymbolUseKind::GlobalConstArraySizeValue,
    SymbolUseKind::GlobalConstVectorElemType,
    SymbolUseKind::GlobalConstMatrixElemType,
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
    SymbolUseKind::GlobalVarValue,      SymbolUseKind::GlobalConstValue,
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
            return out << "global const";
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
        case SymbolUseKind::GlobalConstType:
            return out << "global const type";
        case SymbolUseKind::GlobalConstValue:
            return out << "global const value";
        case SymbolUseKind::GlobalConstArrayElemType:
            return out << "global const array element type";
        case SymbolUseKind::GlobalConstArraySizeValue:
            return out << "global const array size value";
        case SymbolUseKind::GlobalConstVectorElemType:
            return out << "global const vector element type";
        case SymbolUseKind::GlobalConstMatrixElemType:
            return out << "global const matrix element type";
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
        case SymbolUseKind::GlobalConstType:
        case SymbolUseKind::GlobalConstArrayElemType:
        case SymbolUseKind::GlobalConstVectorElemType:
        case SymbolUseKind::GlobalConstMatrixElemType:
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
        case SymbolUseKind::GlobalConstValue:
        case SymbolUseKind::GlobalConstArraySizeValue:
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
        case SymbolUseKind::GlobalConstType:
        case SymbolUseKind::GlobalConstValue:
        case SymbolUseKind::GlobalConstArrayElemType:
        case SymbolUseKind::GlobalConstArraySizeValue:
        case SymbolUseKind::GlobalConstVectorElemType:
        case SymbolUseKind::GlobalConstMatrixElemType:
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
    utils::Vector<const ast::Parameter*, 8> parameters;
    /// Shallow function var / let declaration statements
    utils::Vector<const ast::Statement*, 8> statements;
    /// Nested function local var / let declaration statements
    utils::Vector<const ast::Statement*, 8> nested_statements;
    /// Function attributes
    utils::Vector<const ast::Attribute*, 8> func_attrs;

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
            return b.GlobalVar(source, symbol, b.ty.i32(), ast::AddressSpace::kPrivate);
        case SymbolDeclKind::GlobalConst:
            return b.GlobalConst(source, symbol, b.ty.i32(), b.Expr(1_i));
        case SymbolDeclKind::Alias:
            return b.Alias(source, symbol, b.ty.i32());
        case SymbolDeclKind::Struct:
            return b.Structure(source, symbol, utils::Vector{b.Member("m", b.ty.i32())});
        case SymbolDeclKind::Function:
            return b.Func(source, symbol, utils::Empty, b.ty.void_(), utils::Empty);
        case SymbolDeclKind::Parameter: {
            auto* node = b.Param(source, symbol, b.ty.i32());
            parameters.Push(node);
            return node;
        }
        case SymbolDeclKind::LocalVar: {
            auto* node = b.Var(source, symbol, b.ty.i32());
            statements.Push(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::LocalLet: {
            auto* node = b.Let(source, symbol, b.ty.i32(), b.Expr(1_i));
            statements.Push(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::NestedLocalVar: {
            auto* node = b.Var(source, symbol, b.ty.i32());
            nested_statements.Push(b.Decl(node));
            return node;
        }
        case SymbolDeclKind::NestedLocalLet: {
            auto* node = b.Let(source, symbol, b.ty.i32(), b.Expr(1_i));
            nested_statements.Push(b.Decl(node));
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
            b.GlobalVar(b.Sym(), node, ast::AddressSpace::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.array(node, 4_i), ast::AddressSpace::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.array(b.ty.i32(), node), ast::AddressSpace::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.vec3(node), ast::AddressSpace::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.mat3x4(node), ast::AddressSpace::kPrivate);
            return node;
        }
        case SymbolUseKind::GlobalVarSampledTexElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.sampled_texture(ast::TextureDimension::k2d, node));
            return node;
        }
        case SymbolUseKind::GlobalVarMultisampledTexElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.multisampled_texture(ast::TextureDimension::k2d, node));
            return node;
        }
        case SymbolUseKind::GlobalVarValue: {
            auto* node = b.Expr(source, symbol);
            b.GlobalVar(b.Sym(), b.ty.i32(), ast::AddressSpace::kPrivate, node);
            return node;
        }
        case SymbolUseKind::GlobalConstType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), node, b.Expr(1_i));
            return node;
        }
        case SymbolUseKind::GlobalConstArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.array(node, 4_i), b.Expr(1_i));
            return node;
        }
        case SymbolUseKind::GlobalConstArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.array(b.ty.i32(), node), b.Expr(1_i));
            return node;
        }
        case SymbolUseKind::GlobalConstVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.vec3(node), b.Expr(1_i));
            return node;
        }
        case SymbolUseKind::GlobalConstMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            b.GlobalConst(b.Sym(), b.ty.mat3x4(node), b.Expr(1_i));
            return node;
        }
        case SymbolUseKind::GlobalConstValue: {
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
            b.Structure(b.Sym(), utils::Vector{b.Member("m", node)});
            return node;
        }
        case SymbolUseKind::CallFunction: {
            auto* node = b.Expr(source, symbol);
            statements.Push(b.CallStmt(b.Call(node)));
            return node;
        }
        case SymbolUseKind::ParameterType: {
            auto* node = b.ty.type_name(source, symbol);
            parameters.Push(b.Param(b.Sym(), node));
            return node;
        }
        case SymbolUseKind::LocalVarType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), node)));
            return node;
        }
        case SymbolUseKind::LocalVarArrayElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), b.ty.array(node, 4_u), b.Expr(1_i))));
            return node;
        }
        case SymbolUseKind::LocalVarArraySizeValue: {
            auto* node = b.Expr(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), b.ty.array(b.ty.i32(), node), b.Expr(1_i))));
            return node;
        }
        case SymbolUseKind::LocalVarVectorElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), b.ty.vec3(node))));
            return node;
        }
        case SymbolUseKind::LocalVarMatrixElemType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), b.ty.mat3x4(node))));
            return node;
        }
        case SymbolUseKind::LocalVarValue: {
            auto* node = b.Expr(source, symbol);
            statements.Push(b.Decl(b.Var(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::LocalLetType: {
            auto* node = b.ty.type_name(source, symbol);
            statements.Push(b.Decl(b.Let(b.Sym(), node, b.Expr(1_i))));
            return node;
        }
        case SymbolUseKind::LocalLetValue: {
            auto* node = b.Expr(source, symbol);
            statements.Push(b.Decl(b.Let(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalVarType: {
            auto* node = b.ty.type_name(source, symbol);
            nested_statements.Push(b.Decl(b.Var(b.Sym(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalVarValue: {
            auto* node = b.Expr(source, symbol);
            nested_statements.Push(b.Decl(b.Var(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::NestedLocalLetType: {
            auto* node = b.ty.type_name(source, symbol);
            nested_statements.Push(b.Decl(b.Let(b.Sym(), node, b.Expr(1_i))));
            return node;
        }
        case SymbolUseKind::NestedLocalLetValue: {
            auto* node = b.Expr(source, symbol);
            nested_statements.Push(b.Decl(b.Let(b.Sym(), b.ty.i32(), node)));
            return node;
        }
        case SymbolUseKind::WorkgroupSizeValue: {
            auto* node = b.Expr(source, symbol);
            func_attrs.Push(b.WorkgroupSize(1_i, node, 2_i));
            return node;
        }
    }
    return nullptr;
}

void SymbolTestHelper::Build() {
    auto& b = *builder;
    if (!nested_statements.IsEmpty()) {
        statements.Push(b.Block(nested_statements));
        nested_statements.Clear();
    }
    if (!parameters.IsEmpty() || !statements.IsEmpty() || !func_attrs.IsEmpty()) {
        b.Func("func", parameters, b.ty.void_(), statements, func_attrs);
        parameters.Clear();
        statements.Clear();
        func_attrs.Clear();
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

    Func("A", utils::Empty, ty.void_(), utils::Vector{CallStmt(Call(Expr(Source{{12, 34}}, "B")))});
    Func(Source{{56, 78}}, "B", utils::Empty, ty.void_(), utils::Vector{Return()});

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeConstructed) {
    // fn F() {
    //   { _ = T(); }
    // }
    // type T = i32;

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{Block(Ignore(Construct(ty.type_name(Source{{12, 34}}, "T"))))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedByLocal) {
    // fn F() {
    //   { var v : T; }
    // }
    // type T = i32;

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{Block(Decl(Var("v", ty.type_name(Source{{12, 34}}, "T"))))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedByParam) {
    // fn F(p : T) {}
    // type T = i32;

    Func("F", utils::Vector{Param("p", ty.type_name(Source{{12, 34}}, "T"))}, ty.void_(),
         utils::Empty);
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeUsedAsReturnType) {
    // fn F() -> T {}
    // type T = i32;

    Func("F", utils::Empty, ty.type_name(Source{{12, 34}}, "T"), utils::Empty);
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, TypeByStructMember) {
    // struct S { m : T };
    // type T = i32;

    Structure("S", utils::Vector{Member("m", ty.type_name(Source{{12, 34}}, "T"))});
    Alias(Source{{56, 78}}, "T", ty.i32());

    Build();
}

TEST_F(ResolverDependencyGraphUsedBeforeDeclTest, VarUsed) {
    // fn F() {
    //   { G = 3.14f; }
    // }
    // var G: f32 = 2.1;

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{
             Block(Assign(Expr(Source{{12, 34}}, "G"), 3.14_f)),
         });

    GlobalVar(Source{{56, 78}}, "G", ty.f32(), ast::AddressSpace::kPrivate, Expr(2.1_f));

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
    GlobalVar(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123_i));
    Build(R"(error: cyclic dependency found: 'SYMBOL' -> 'SYMBOL'
12:34 note: var 'SYMBOL' references var 'SYMBOL' here)");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, GlobalConst) {
    const Symbol symbol = Sym("SYMBOL");
    GlobalConst(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123_i));
    Build(R"(error: cyclic dependency found: 'SYMBOL' -> 'SYMBOL'
12:34 note: const 'SYMBOL' references const 'SYMBOL' here)");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, LocalVar) {
    const Symbol symbol = Sym("SYMBOL");
    WrapInFunction(Decl(Var(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123_i))));
    Build("12:34 error: unknown identifier: 'SYMBOL'");
}

TEST_F(ResolverDependencyGraphDeclSelfUse, LocalLet) {
    const Symbol symbol = Sym("SYMBOL");
    WrapInFunction(Decl(Let(symbol, ty.i32(), Mul(Expr(Source{{12, 34}}, symbol), 123_i))));
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

    Func(Source{{12, 34}}, "main", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Expr(Source{{56, 78}}, "main")))});

    Build(R"(12:34 error: cyclic dependency found: 'main' -> 'main'
56:78 note: function 'main' calls function 'main' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, IndirectCall) {
    // 1: fn a() { b(); }
    // 2: fn e() { }
    // 3: fn d() { e(); b(); }
    // 4: fn c() { d(); }
    // 5: fn b() { c(); }

    Func(Source{{1, 1}}, "a", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Expr(Source{{1, 10}}, "b")))});
    Func(Source{{2, 1}}, "e", utils::Empty, ty.void_(), utils::Empty);
    Func(Source{{3, 1}}, "d", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call(Expr(Source{{3, 10}}, "e"))),
             CallStmt(Call(Expr(Source{{3, 10}}, "b"))),
         });
    Func(Source{{4, 1}}, "c", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Expr(Source{{4, 10}}, "d")))});
    Func(Source{{5, 1}}, "b", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Expr(Source{{5, 10}}, "c")))});

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

    Structure(Source{{12, 34}}, "S",
              utils::Vector{Member("a", ty.type_name(Source{{56, 78}}, "S"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'S' -> 'S'
56:78 note: struct 'S' references struct 'S' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Struct_Indirect) {
    // 1: struct Y { z: Z; };
    // 2: struct X { y: Y; };
    // 3: struct Z { x: X; };

    Structure(Source{{1, 1}}, "Y", utils::Vector{Member("z", ty.type_name(Source{{1, 10}}, "Z"))});
    Structure(Source{{2, 1}}, "X", utils::Vector{Member("y", ty.type_name(Source{{2, 10}}, "Y"))});
    Structure(Source{{3, 1}}, "Z", utils::Vector{Member("x", ty.type_name(Source{{3, 10}}, "X"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: struct 'Y' references struct 'Z' here
3:10 note: struct 'Z' references struct 'X' here
2:10 note: struct 'X' references struct 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalVar_Direct) {
    // var<private> V : i32 = V;

    GlobalVar(Source{{12, 34}}, "V", ty.i32(), Expr(Source{{56, 78}}, "V"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'V' -> 'V'
56:78 note: var 'V' references var 'V' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalConst_Direct) {
    // let V : i32 = V;

    GlobalConst(Source{{12, 34}}, "V", ty.i32(), Expr(Source{{56, 78}}, "V"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cyclic dependency found: 'V' -> 'V'
56:78 note: const 'V' references const 'V' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalVar_Indirect) {
    // 1: var<private> Y : i32 = Z;
    // 2: var<private> X : i32 = Y;
    // 3: var<private> Z : i32 = X;

    GlobalVar(Source{{1, 1}}, "Y", ty.i32(), Expr(Source{{1, 10}}, "Z"));
    GlobalVar(Source{{2, 1}}, "X", ty.i32(), Expr(Source{{2, 10}}, "Y"));
    GlobalVar(Source{{3, 1}}, "Z", ty.i32(), Expr(Source{{3, 10}}, "X"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: var 'Y' references var 'Z' here
3:10 note: var 'Z' references var 'X' here
2:10 note: var 'X' references var 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, GlobalConst_Indirect) {
    // 1: const Y : i32 = Z;
    // 2: const X : i32 = Y;
    // 3: const Z : i32 = X;

    GlobalConst(Source{{1, 1}}, "Y", ty.i32(), Expr(Source{{1, 10}}, "Z"));
    GlobalConst(Source{{2, 1}}, "X", ty.i32(), Expr(Source{{2, 10}}, "Y"));
    GlobalConst(Source{{3, 1}}, "Z", ty.i32(), Expr(Source{{3, 10}}, "X"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(1:1 error: cyclic dependency found: 'Y' -> 'Z' -> 'X' -> 'Y'
1:10 note: const 'Y' references const 'Z' here
3:10 note: const 'Z' references const 'X' here
2:10 note: const 'X' references const 'Y' here)");
}

TEST_F(ResolverDependencyGraphCyclicRefTest, Mixed_RecursiveDependencies) {
    // 1: fn F() -> R { return Z; }
    // 2: type A = S;
    // 3: struct S { a : A };
    // 4: var Z = L;
    // 5: type R = A;
    // 6: const L : S = Z;

    Func(Source{{1, 1}}, "F", utils::Empty, ty.type_name(Source{{1, 5}}, "R"),
         utils::Vector{Return(Expr(Source{{1, 10}}, "Z"))});
    Alias(Source{{2, 1}}, "A", ty.type_name(Source{{2, 10}}, "S"));
    Structure(Source{{3, 1}}, "S", utils::Vector{Member("a", ty.type_name(Source{{3, 10}}, "A"))});
    GlobalVar(Source{{4, 1}}, "Z", Expr(Source{{4, 10}}, "L"));
    Alias(Source{{5, 1}}, "R", ty.type_name(Source{{5, 10}}, "A"));
    GlobalConst(Source{{6, 1}}, "L", ty.type_name(Source{{5, 5}}, "S"), Expr(Source{{5, 10}}, "Z"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(2:1 error: cyclic dependency found: 'A' -> 'S' -> 'A'
2:10 note: alias 'A' references struct 'S' here
3:10 note: struct 'S' references alias 'A' here
4:1 error: cyclic dependency found: 'Z' -> 'L' -> 'Z'
4:10 note: var 'Z' references const 'L' here
5:10 note: const 'L' references var 'Z' here)");
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

    ASSERT_EQ(AST().GlobalDeclarations().Length(), 2u);

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

    ASSERT_EQ(AST().GlobalDeclarations().Length(), 2u);

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

TEST_F(ResolverDependencyGraphOrderedGlobalsTest, EnableFirst) {
    // Test that enable nodes always go before any other global declaration.
    // Although all enable directives in a valid WGSL program must go before any other global
    // declaration, a transform may produce such a AST tree that has some declarations before enable
    // nodes. DependencyGraph should deal with these cases.
    auto* var_1 = GlobalVar("SYMBOL1", ty.i32());
    auto* enable_1 = Enable(ast::Extension::kF16);
    auto* var_2 = GlobalVar("SYMBOL2", ty.f32());
    auto* enable_2 = Enable(ast::Extension::kF16);

    EXPECT_THAT(AST().GlobalDeclarations(), ElementsAre(var_1, enable_1, var_2, enable_2));
    EXPECT_THAT(Build().ordered_globals, ElementsAre(enable_1, enable_2, var_1, var_2));
}
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
        auto resolved_symbol = graph.resolved_symbols.Find(use);
        ASSERT_TRUE(resolved_symbol);
        EXPECT_EQ(*resolved_symbol, decl)
            << "resolved: " << (*resolved_symbol ? (*resolved_symbol)->TypeInfo().name : "<null>")
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
    auto* inner_var = helper.nested_statements.Length()
                          ? helper.nested_statements[0]->As<ast::VariableDeclStatement>()->variable
                      : helper.statements.Length()
                          ? helper.statements[0]->As<ast::VariableDeclStatement>()->variable
                          : helper.parameters[0];
    helper.Build();

    auto shadows = Build().shadows;
    auto shadow = shadows.Find(inner_var);
    ASSERT_TRUE(shadow);
    EXPECT_EQ(*shadow, outer);
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

    const auto* value_decl = GlobalVar(value_sym, ty.i32(), ast::AddressSpace::kPrivate);
    const auto* type_decl = Alias(type_sym, ty.i32());
    const auto* func_decl = Func(func_sym, utils::Empty, ty.void_(), utils::Empty);

    struct SymbolUse {
        const ast::Node* decl = nullptr;
        const ast::Node* use = nullptr;
        std::string where;
    };

    utils::Vector<SymbolUse, 64> symbol_uses;

    auto add_use = [&](const ast::Node* decl, auto* use, int line, const char* kind) {
        symbol_uses.Push(
            SymbolUse{decl, use, std::string(__FILE__) + ":" + std::to_string(line) + ": " + kind});
        return use;
    };
#define V add_use(value_decl, Expr(value_sym), __LINE__, "V()")
#define T add_use(type_decl, ty.type_name(type_sym), __LINE__, "T()")
#define F add_use(func_decl, Expr(func_sym), __LINE__, "F()")

    Alias(Sym(), T);
    Structure(Sym(),  //
              utils::Vector{Member(Sym(), T,
                                   utils::Vector{
                                       //
                                       MemberAlign(V), MemberSize(V)  //
                                   })});
    GlobalVar(Sym(), T, V);
    GlobalConst(Sym(), T, V);
    Func(Sym(),
         utils::Vector{
             Param(Sym(), T,
                   utils::Vector{
                       Location(V),  // Parameter attributes
                   }),
         },
         T,  // Return type
         utils::Vector{
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
             While(Equal(V, V),                         //
                   Block(                               //
                       Assign(V, V))),                  //
             Loop(Block(Assign(V, V)),                  //
                  Block(Assign(V, V), BreakIf(V))),     //
             Switch(V,                                  //
                    Case(CaseSelector(1_i),             //
                         Block(Assign(V, V))),          //
                    DefaultCase(Block(Assign(V, V)))),  //
             Return(V),                                 //
             Break(),                                   //
             Discard(),                                 //
         },
         utils::Empty,                 // function attributes
         utils::Vector{Location(V)});  // return attributes
    // Exercise type traversal
    GlobalVar(Sym(), ty.atomic(T));
    GlobalVar(Sym(), ty.bool_());
    GlobalVar(Sym(), ty.i32());
    GlobalVar(Sym(), ty.u32());
    GlobalVar(Sym(), ty.f32());
    GlobalVar(Sym(), ty.array(T, V, 4));
    GlobalVar(Sym(), ty.vec3(T));
    GlobalVar(Sym(), ty.mat3x2(T));
    GlobalVar(Sym(), ty.pointer(T, ast::AddressSpace::kPrivate));
    GlobalVar(Sym(), ty.sampled_texture(ast::TextureDimension::k2d, T));
    GlobalVar(Sym(), ty.depth_texture(ast::TextureDimension::k2d));
    GlobalVar(Sym(), ty.depth_multisampled_texture(ast::TextureDimension::k2d));
    GlobalVar(Sym(), ty.external_texture());
    GlobalVar(Sym(), ty.multisampled_texture(ast::TextureDimension::k2d, T));
    GlobalVar(Sym(), ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                        ast::Access::kRead));  //
    GlobalVar(Sym(), ty.sampler(ast::SamplerKind::kSampler));

    GlobalVar(Sym(), ty.i32(), utils::Vector{Binding(V), Group(V)});
    GlobalVar(Sym(), ty.i32(), utils::Vector{Location(V)});
    Override(Sym(), ty.i32(), utils::Vector{Id(V)});

    Func(Sym(), utils::Empty, ty.void_(), utils::Empty);
#undef V
#undef T
#undef F

    auto graph = Build();
    for (auto use : symbol_uses) {
        auto resolved_symbol = graph.resolved_symbols.Find(use.use);
        ASSERT_TRUE(resolved_symbol) << use.where;
        EXPECT_EQ(*resolved_symbol, use.decl) << use.where;
    }
}

TEST_F(ResolverDependencyGraphTraversalTest, InferredType) {
    // Check that the nullptr of the var / const / let type doesn't make things explode
    GlobalVar("a", Expr(1_i));
    GlobalConst("b", Expr(1_i));
    WrapInFunction(Var("c", Expr(1_i)),  //
                   Let("d", Expr(1_i)));
    Build();
}

// Reproduces an unbalanced stack push / pop bug in
// DependencyAnalysis::SortGlobals(), found by clusterfuzz.
// See: crbug.com/chromium/1273451
TEST_F(ResolverDependencyGraphTraversalTest, chromium_1273451) {
    Structure("A", utils::Vector{Member("a", ty.i32())});
    Structure("B", utils::Vector{Member("b", ty.i32())});
    Func("f", utils::Vector{Param("a", ty.type_name("A"))}, ty.type_name("B"),
         utils::Vector{
             Return(Construct(ty.type_name("B"))),
         });
    Build();
}

}  // namespace ast_traversal

}  // namespace
}  // namespace tint::resolver
