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

#ifndef SRC_TINT_RESOLVER_SEM_HELPER_H_
#define SRC_TINT_RESOLVER_SEM_HELPER_H_

#include <string>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/utils/map.h"

namespace tint::resolver {

/// Helper class to retrieve sem information.
class SemHelper {
  public:
    /// Constructor
    /// @param builder the program builder
    /// @param dependencies the program dependency graph
    explicit SemHelper(ProgramBuilder* builder, DependencyGraph& dependencies);
    ~SemHelper();

    /// Get is a helper for obtaining the semantic node for the given AST node.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for the provided |ast|
    template <typename SEM = sem::Info::InferFromAST, typename AST_OR_TYPE = CastableBase>
    auto* Get(const AST_OR_TYPE* ast) const {
        using T = sem::Info::GetResultType<SEM, AST_OR_TYPE>;
        auto* sem = builder_->Sem().Get(ast);
        if (!sem) {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "AST node '" << ast->TypeInfo().name << "' had no semantic info\n"
                << "At: " << ast->source << "\n"
                << "Pointer: " << ast;
        }
        return const_cast<T*>(As<T>(sem));
    }

    /// @returns the resolved symbol (function, type or variable) for the given
    /// ast::Identifier or ast::TypeName cast to the given semantic type.
    /// @param node the node to retrieve
    template <typename SEM = sem::Node>
    SEM* ResolvedSymbol(const ast::Node* node) const {
        auto* resolved = utils::Lookup(dependencies_.resolved_symbols, node);
        return resolved ? const_cast<SEM*>(builder_->Sem().Get<SEM>(resolved)) : nullptr;
    }

    /// @returns the resolved type of the ast::Expression `expr`
    /// @param expr the expression
    sem::Type* TypeOf(const ast::Expression* expr) const;

    /// @returns the semantic type of the AST literal `lit`
    /// @param lit the literal
    sem::Type* TypeOf(const ast::LiteralExpression* lit);

    /// @returns the type name of the given semantic type, unwrapping
    /// references.
    /// @param ty the type to look up
    std::string TypeNameOf(const sem::Type* ty) const;

    /// @returns the type name of the given semantic type, without unwrapping
    /// references.
    /// @param ty the type to look up
    std::string RawTypeNameOf(const sem::Type* ty) const;

  private:
    ProgramBuilder* builder_;
    DependencyGraph& dependencies_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_SEM_HELPER_H_
