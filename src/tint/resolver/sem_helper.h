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
    /// Raises an ICE and returns `nullptr` if there is no semantic node associated with the AST
    /// node.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename SEM = sem::Info::InferFromAST, typename AST = ast::Node>
    auto* Get(const AST* ast) const {
        using T = sem::Info::GetResultType<SEM, AST>;
        auto* sem = builder_->Sem().Get(ast);
        if (TINT_UNLIKELY(!sem)) {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "AST node '" << ast->TypeInfo().name << "' had no semantic info\n"
                << "At: " << ast->source << "\n"
                << "Pointer: " << ast;
        }
        return const_cast<T*>(As<T>(sem));
    }

    /// GetVal is a helper for obtaining the semantic sem::ValueExpression for the given AST node.
    /// Raises an error diagnostic and returns `nullptr` if the semantic node is not a
    /// sem::ValueExpression.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename AST = ast::Node>
    auto* GetVal(const AST* ast) const {
        if constexpr (traits::IsTypeOrDerived<sem::SemanticNodeTypeFor<AST>,
                                              sem::ValueExpression>) {
            return Get(ast);
        } else {
            if (auto* sem = Get(ast); TINT_LIKELY(sem)) {
                auto* val = sem->template As<sem::ValueExpression>();
                if (TINT_LIKELY(val)) {
                    return val;
                }
                // TODO(crbug.com/tint/1810): Improve error
                builder_->Diagnostics().add_error(diag::System::Resolver,
                                                  "required value expression, got something else",
                                                  ast->source);
            }
            return static_cast<sem::ValueExpression*>(nullptr);
        }
    }

    /// @returns the resolved symbol (function, type or variable) for the given ast::Identifier or
    /// ast::TypeName cast to the given semantic type.
    /// @param node the node to retrieve
    template <typename SEM = sem::Info::InferFromAST>
    sem::Info::GetResultType<SEM, ast::Node>* ResolvedSymbol(const ast::Node* node) const {
        if (auto resolved = dependencies_.resolved_symbols.Find(node)) {
            auto* sem = builder_->Sem().Get<SEM>(*resolved);
            return const_cast<sem::Info::GetResultType<SEM, ast::Node>*>(sem);
        }
        return nullptr;
    }

    /// @returns the resolved type of the ast::Expression `expr`
    /// @param expr the expression
    type::Type* TypeOf(const ast::Expression* expr) const;

    /// @returns the type name of the given semantic type, unwrapping
    /// references.
    /// @param ty the type to look up
    std::string TypeNameOf(const type::Type* ty) const;

    /// @returns the type name of the given semantic type, without unwrapping
    /// references.
    /// @param ty the type to look up
    std::string RawTypeNameOf(const type::Type* ty) const;

  private:
    ProgramBuilder* builder_;
    DependencyGraph& dependencies_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_SEM_HELPER_H_
